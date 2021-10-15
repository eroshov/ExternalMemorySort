#pragma once
#include <concepts>
#include <iterator>

namespace detail {
template <class T>
concept Iterable = requires(T x) {
  x.begin();
  x.end();
  std::next(x.begin());
};

} // namespace detail