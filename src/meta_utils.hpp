#pragma once
#include <type_traits>

namespace detail {

enum class DataType {
  kDefault,
  kContainer
};

using DefaultType = std::integral_constant<DataType, DataType::kDefault>;
using ContainerType = std::integral_constant<DataType, DataType::kContainer>;

template <class T, class = void>
struct DataTypeConstructor : DefaultType {};

template <class T>
struct DataTypeConstructor <
  T,
  std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>
> : ContainerType {};

} // namespace detail