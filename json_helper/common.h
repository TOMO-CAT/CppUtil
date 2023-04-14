#pragma once

#include <type_traits>

namespace json_helper {

#ifndef _JSON_HELPER_DEBUG
#define _JSON_HELPER_DEBUG false
#endif

constexpr char kNullptrJsonStr[] = "nullptr";

// 辅助类: 用于判断 class 是否包含 Unmarshal 成员函数
template <typename T, typename = std::void_t<>>
struct HasUnmarshalFunc : std::false_type {};
template <typename T>
struct HasUnmarshalFunc<T, std::void_t<decltype(&T::Unmarshal)>> : std::true_type {};

// 辅助类: 用于判断 class 是否包含 Marshal 函数
template <typename T, typename = std::void_t<>>
struct HasMarshalFunc : std::false_type {};
template <typename T>
struct HasMarshalFunc<T, std::void_t<decltype(&T::Marshal)>> : std::true_type {};

}  // namespace json_helper