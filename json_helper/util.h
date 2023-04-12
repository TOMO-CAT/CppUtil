#pragma once

#include <type_traits>

namespace json_helper {

// 辅助类: 用于判断 class 是否包含 Unmarshal 成员函数
template <typename T, typename = std::void_t<>>
struct HasUnmarshalFunc : std::false_type {};
template <typename T>
struct HasUnmarshalFunc<T, std::void_t<decltype(&T::Unmarshal)>> : std::true_type {};

// 辅助类: 用于判断是否是 enum class
template <typename T, typename = std::void_t<>>
struct IsEnumClass : std::false_type {};

template <typename T>
struct IsEnumClass<T, typename std::enable_if<std::is_enum<T>::value && !std::is_convertible<T, int>::value>::type>
    : std::true_type {};

}  // namespace json_helper