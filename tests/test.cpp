#include <gtest/gtest.h>
#include <file_utils.hpp>
#include <sort_performer.hpp>
#include <numeric>

namespace {

static constexpr auto input = "input";
static constexpr auto output = "output";

void WriteDataToBinaryFile(const std::vector<unsigned>& data) {
  std::ofstream os(input, std::ios::binary);
  file_utils::WriteBinary(os, data);
}

std::vector<unsigned> ReadDataFromBinaryFile(std::size_t size) {
  std::ifstream is(output, std::ios::binary);
  std::vector<unsigned> data(size);
  file_utils::ReadBinary(is, data);
  return data;
}

} // namespace

TEST(SortPerformerSimpleTests, NotEnoughMemory) {
  try {
    sort::SortPerformer sort_performer(8 << 10, BUFSIZ);
  } catch (std::runtime_error& exc) {
    return;
  }
  EXPECT_EQ(0, 1);
}

TEST(SortPerformerSimpleTests, EqualElements) {
  const auto size = 2 << 20;
  std::vector<unsigned> data(size, 0);
  WriteDataToBinaryFile(data);

  sort::SortPerformer sort_performer(1 << 20, BUFSIZ);
  sort_performer.Sort(input, output);

  EXPECT_EQ(data, ReadDataFromBinaryFile(size));
}

TEST(SortPerformerSimpleTests, InternalSort) {
  const auto size = 1 << 20;
  std::vector<unsigned> data(size, 0);
  WriteDataToBinaryFile(data);

  sort::SortPerformer sort_performer(4 << 20, BUFSIZ);
  sort_performer.Sort(input, output);

  EXPECT_EQ(data, ReadDataFromBinaryFile(size));
}

TEST(SortPerformerSimpleTests, ExternalSort) {
  const auto size = 4 << 20;
  std::vector<unsigned> data(size);
  std::iota(data.rbegin(), data.rend(), 0);
  WriteDataToBinaryFile(data);

  sort::SortPerformer sort_performer(1 << 20, BUFSIZ);
  sort_performer.Sort(input, output);

  std::reverse(data.begin(), data.end());
  EXPECT_EQ(data, ReadDataFromBinaryFile(size));
}

TEST(SortPerformerSimpleTests, CleanUp) {
  std::remove(input);
  std::remove(output);
}