#include "sort_performer.hpp"
#include "file_utils.hpp"
#include <atomic>
#include <cstdio>
#include <future>

namespace sort {

SortPerformer::SortPerformer(std::size_t main_memory_capacity, std::size_t buffer_size)
  : main_memory_total_capacity_(main_memory_capacity),
    buffer_size_(buffer_size),
    max_num_streams_(MaxNumStreams()),
    input_streams_(MaxNumStreams()),
    output_stream_(),
    data_priority_queue_() {
  if (MainMemoryAvailableCapacity() == 0 || MaxNumStreams() == 0) {
    throw std::runtime_error("Not enough memory for SortPerformer ^_^\n"
                             "You can increase main memory capacity or decrease buffer size\n");
  }
}

void SortPerformer::Sort(std::string_view input, std::string_view output) {
  const auto num_sorted_chunks = SortSmallestChunksPar(input);

  if (num_sorted_chunks == 0) { // empty input implies empty output
    std::ofstream(output, std::ios::binary);
    return;
  }

  if (num_sorted_chunks > 1) {
    while (Merge() > 1);
  }

  std::rename("0", output.data());
}

std::size_t SortPerformer::MainMemoryAvailableCapacity() const {
  const auto available_capacity = [this]() {
    const double memory_efficiency_factor = 64. / (buffer_size_ + 64);
    const std::size_t internal_needs =
        main_memory_total_capacity_ * memory_efficiency_factor + (8 << 10);
    return main_memory_total_capacity_ > internal_needs ?
        main_memory_total_capacity_ - internal_needs : 0;
  }();
  return available_capacity;
}

std::size_t SortPerformer::MaxNumStreams() const {
  const auto res = std::min(512ul, MainMemoryAvailableCapacity() / buffer_size_);
  return res;
}

std::size_t SortPerformer::NumIOThreads() const {
  // IO operations do not benefit much from num of threads larger than 4
  const auto num_io_threads = []() {
    const auto num_available_threads = std::max(std::thread::hardware_concurrency(), 2u);
    return std::min(num_available_threads, 4u);
  }();
  return num_io_threads;
}

std::size_t SortPerformer::SortSmallestChunks(std::string_view input) const {
  const std::size_t max_chunk_size = MainMemoryAvailableCapacity() / sizeof(unsigned);
  std::vector<unsigned> numbers(max_chunk_size);
  std::ifstream is(input, std::ios_base::binary);

  std::size_t num_chunks = 0;
  for(;; ++num_chunks) {
    numbers.resize(max_chunk_size);
    const auto chunk_size = file_utils::ReadBinary(is, numbers);
    if (chunk_size == 0) break;

    numbers.resize(chunk_size);
    std::sort(numbers.begin(), numbers.end());
    std::ofstream os(std::to_string(num_chunks));
    file_utils::WriteBinary(os, numbers);
  }
  return num_chunks;
}

std::size_t SortPerformer::SortSmallestChunksPar(std::string_view input) {
  const std::size_t num_threads = NumIOThreads();
  const std::size_t max_chunk_size =
      MainMemoryAvailableCapacity() / sizeof(unsigned) / num_threads;

  std::vector<std::vector<unsigned>> data(num_threads);
  std::vector<std::future<std::size_t>> num_sorted_chunks_in_thread(num_threads);

  std::size_t num_output_files = 0;
  std::size_t num_input_files = 0;

  std::size_t input_offset = 0;
  const std::size_t offset_step = max_chunk_size * sizeof(unsigned);

  for(;;) {
    for (std::size_t thread_i = 0; thread_i < num_threads; ++thread_i) {
      std::launch policy = thread_i == num_threads - 1 ?
          std::launch::any : std::launch::async;

      num_sorted_chunks_in_thread[thread_i] = std::async(
          policy, &SortPerformer::SortSmallestChunk,
          this, input, std::ref(data[thread_i]), max_chunk_size, input_offset, num_input_files);

      ++num_input_files;
      input_offset += offset_step;
    }

    std::size_t num_sorted_chunks = 0;
    for (auto& future : num_sorted_chunks_in_thread) {
      num_sorted_chunks += future.get();
    }

    num_output_files += num_sorted_chunks;
    if (num_sorted_chunks < num_threads) break;
  }

  return num_output_files;
}

std::size_t SortPerformer::SortSmallestChunk(
    std::string_view input, std::vector<unsigned>& data, std::size_t max_chunk_size,
    std::size_t input_offset, std::size_t output_file_num) {
  std::ifstream is(input, std::ios_base::binary);
  is.seekg(input_offset, std::ios_base::beg);
  data.resize(max_chunk_size);
  const auto chunk_size = file_utils::ReadBinary(is, data);

  if (chunk_size == 0) return 0;

  data.resize(chunk_size);
  std::sort(data.begin(), data.end());
  std::ofstream os(std::to_string(output_file_num));
  file_utils::WriteBinary(os, data);

  return 1;
}

std::size_t SortPerformer::OpenInputStreams(std::size_t first) {
  std::size_t num_opened_streams = 0;
  input_streams_.resize(max_num_streams_);
  for (; num_opened_streams < max_num_streams_; ++num_opened_streams) {
    const auto stream_name = std::to_string(first + num_opened_streams);
    auto& stream = input_streams_[num_opened_streams];

    stream.open(stream_name, std::ios::binary);
    if (file_utils::HasFailbitSet(stream)) {
      break;
    }
  }
  input_streams_.resize(num_opened_streams);
  return num_opened_streams;
}

void SortPerformer::CloseAndRemoveInputStreams(std::size_t first) {
  for (std::size_t i = 0; i < input_streams_.size(); ++i) {
    input_streams_[i].close();
    std::remove(std::to_string(first + i).c_str());
  }
}

void SortPerformer::PopulateQueueFromInputStreams() {
  // Build heap from vector to gain O(n) time complexity
  std::vector<DataAndFile> heap_initial_data(input_streams_.size());
  for (std::size_t i = 0; i < input_streams_.size(); ++i) {
    const auto data = file_utils::ReadBinary<unsigned>(input_streams_[i]);
    heap_initial_data[i] = std::make_pair(data, i);
  }
  data_priority_queue_ = {heap_initial_data.begin(), heap_initial_data.end()};
}

void SortPerformer::MergeInputStreams(std::string_view output) {
  output_stream_.open(output);

  while (!data_priority_queue_.empty()) {
    const auto [data, input_stream_index] = data_priority_queue_.top();
    data_priority_queue_.pop();
    auto& input_stream = input_streams_[input_stream_index];

    file_utils::WriteBinary(output_stream_, data);

    const auto new_data = file_utils::ReadBinary<unsigned>(input_stream);
    if (!file_utils::HasEofbitSet(input_stream)) {
      data_priority_queue_.push(std::make_pair(new_data, input_stream_index));
    }
  }

  output_stream_.close();
}

std::size_t SortPerformer::Merge() {
  std::size_t num_output_streams = 0;
  std::size_t num_input_streams = 0;

  for (;; ++num_output_streams) {
    const auto num_opened_streams = OpenInputStreams(num_input_streams);
    if (num_opened_streams == 0) break;
    PopulateQueueFromInputStreams();

    std::string output = "_" + std::to_string(num_output_streams);
    MergeInputStreams(output);

    CloseAndRemoveInputStreams(num_input_streams);
    std::rename(output.c_str(), std::to_string(num_output_streams).c_str());
    num_input_streams += num_opened_streams;
  }
  return num_output_streams;
}

} // namespace sort