#pragma once

#include <array>
#include <string_view>
#include <type_traits>
#include <utility>

/**
 * @Reference https://zhuanlan.zhihu.com/p/680412313
 *
 */

namespace util {

template <auto value>
constexpr auto enum_name() {
  std::string_view name;
#ifdef __clang__
  name = __PRETTY_FUNCTION__;
  auto start = name.find("value = ") + 8;  // 8 is length of "value = "
  auto end = name.find_last_of(']');
  return std::string_view{name.data() + start, end - start};

#elif defined(__GNUC__)
  name = __PRETTY_FUNCTION__;
  auto start = name.find("value = ") + 8;  // 8 is length of "value = "
  auto end = name.find_last_of(']');
  return std::string_view{name.data() + start, end - start};

#elif defined(_MSC_VER)
  name = __FUNCSIG__;
  auto start = name.find("enum_name<") + 10;  // 10 is length of "enum_name<"
  auto end = name.find_last_of('>');
  return std::string_view{name.data() + start, end - start};
#endif
}

template <typename T, std::size_t N = 0>
constexpr auto enum_max() {
  constexpr auto value = static_cast<T>(N);
  if constexpr (enum_name<value>().find(")") == std::string_view::npos)
    return enum_max<T, N + 1>();
  else
    return N;
}

template <typename T>
constexpr auto enum_name(T value) {
  static_assert(std::is_enum<T>::value, "T must be an enum type");
  constexpr auto num = enum_max<T>();
  constexpr auto names = []<std::size_t... Is>(std::index_sequence<Is...>) {
    return std::array<std::string_view, num>{enum_name<static_cast<T>(Is)>()...};
  }
  (std::make_index_sequence<num>{});
  return names[static_cast<std::size_t>(value)];
}

}  // namespace util
