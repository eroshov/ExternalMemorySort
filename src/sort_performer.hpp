#pragma once

#include <iostream>
#include <fstream>
#include <queue>
#include <string_view>
#include <thread>
#include <vector>

namespace sort {

class SortPerformer {
 public:
  SortPerformer(std::size_t main_memory_capacity, std::size_t buffer_size);

  void Sort(std::string_view input, std::string_view output);

 private:
  using DataAndFile = std::pair<unsigned, std::size_t>;

  std::size_t MainMemoryAvailableCapacity() const;
  std::size_t MaxNumStreams() const;
  std::size_t NumIOThreads() const;

  std::size_t OpenInputStreams(std::size_t first);
  void CloseAndRemoveInputStreams(std::size_t first);
  void PopulateQueueFromInputStreams();
  void MergeInputStreams(std::string_view output);

  std::size_t SortSmallestChunksPar(std::string_view input);
  std::size_t SortSmallestChunk(
      std::string_view input, std::vector<unsigned>& data,
      std::size_t max_chunk_size, std::size_t input_offset, std::size_t output_file_num);
  std::size_t SortSmallestChunks(std::string_view input) const;
  std::size_t Merge();

  /// Size of main memory in bytes that is available for algorithm including internal DS
  const std::size_t main_memory_total_capacity_;
  /// Size of buffer allocated for each stream
  const std::size_t buffer_size_;
  /// Maximum number of chunks for k-way merge. Each chunk is represented by input stream
  const std::size_t max_num_streams_;

  std::vector<std::ifstream> input_streams_;
  std::ofstream output_stream_;
  std::priority_queue<DataAndFile, std::vector<DataAndFile>, std::greater<>> data_priority_queue_;
};

} // namespace sort