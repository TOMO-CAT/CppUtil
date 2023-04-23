#pragma once

#include <string>

#include "json/json.h"
#include "json_helper/marshal.h"
#include "json_helper/unmarshal.h"

namespace json_helper {

template <typename T>
bool Unmarshal(const std::string& str, T* const obj) {
  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(str, root)) {
#ifndef NDEBUG
    std::cout << "[JsonHelper][Warning] jsoncpp parse fail, str: " << str << std::endl;
#endif
    return false;
  }
  return Unmarshal(root, obj);
}

template <typename T>
bool Unmarshal(const char* str, T* const obj) {
  return Unmarshal(std::string(str), obj);
}

template <typename T>
bool Marshal(const T& obj, std::string* const json_str, bool with_style = false) {
  Json::Value root;

  // 允许部分字段序列化失败
  bool ret = Marshal(obj, &root);
  // if (!Marshal(obj, &root)) {
  //     *json_str = "";
  //     return false;
  // }

  if (with_style) {
    Json::StyledWriter writer;
    *json_str = writer.write(root);
  } else {
    Json::FastWriter writer;
    *json_str = writer.write(root);
  }

  return ret;
}

template <typename T>
std::string ToString(const T& obj) {
  std::string res;
  if (!Marshal(obj, &res, false)) {
    return "";
  }
  return res;
}

#define JSON_HELPER(...) \
  JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(__VA_ARGS__) JSON_HELPER_MARSHAL_MEMBER_FUNCTION(__VA_ARGS__)

}  // namespace json_helper
