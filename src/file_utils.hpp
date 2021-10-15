#pragma once
#include <fstream>
#include "meta_utils.hpp"

namespace file_utils {

bool HasEofbitSet(const std::istream& is);
bool HasFailbitSet(const std::istream& is);

template <class T>
T ReadBinary(std::istream& is) {
  T data;
  is.read(reinterpret_cast<char *>(&data), sizeof(T));
  return data;
}

template <class T>
std::size_t ReadBinary(std::istream& is, T& data) {
  is.read(reinterpret_cast<char *>(&data), sizeof(T));
  return 1;
}

template <class T>
void WriteBinary(std::ostream& os, const T& data) {
  os.write(reinterpret_cast<const char *>(&data), sizeof(data));
}

template <detail::Iterable T>
std::size_t ReadBinary(std::istream& is, T& container) {
  std::size_t num_items = 0;
  for (auto it = container.begin(); it != container.end(); ++it, ++num_items) {
    ReadBinary(is, *it);
    if (file_utils::HasEofbitSet(is)) break;
  }
  return num_items;
}

template <detail::Iterable T>
void WriteBinary(std::ostream& os, const T& container) {
  for (const auto& item : container) {
    WriteBinary(os, item);
  }
}

} // namespace file_utils