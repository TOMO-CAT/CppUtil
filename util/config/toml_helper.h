#pragma once

#include <memory>
#include <sstream>
#include <string>

#include "cpptoml/cpptoml.h"

namespace util {

/*
 * 获取TOML配置中的值, 失败返回false
 * */
template <typename T>
bool ParseTomlValue(std::shared_ptr<cpptoml::table> g, const std::string& key, T* value) {
  auto val = g->get_qualified_as<T>(key);
  if (!val) {
    // log_error("parse key fail, key: %s", key.c_str());
    return false;
  }
  *value = *val;

  std::ostringstream oss;
  oss << value;
  // log_info("parse toml succ, key:%s value:%s", key.c_str(), oss.str().c_str());
  return true;
}

}  // namespace util
