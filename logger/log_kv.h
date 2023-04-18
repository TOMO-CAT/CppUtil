#pragma once

#include <sstream>
#include <string>

#include "logger/logger.h"

namespace logger {

class LoggerKV {
 public:
  LoggerKV(const Logger::Level level, const std::string& file, const uint32_t line, const std::string& function,
           const std::string& prefix);
  ~LoggerKV();

 public:
  template <typename T>
  LoggerKV& LogKV(const std::string& key, const T& val) {
    sstream_ << kSeparator << key << "=" << val;
    return *this;
  }

  LoggerKV& LogKVFormat(const char* const format, ...);

 private:
  static constexpr char kSeparator[] = "||";
  std::ostringstream sstream_;

  Logger::Level level_;
  std::string file_;
  uint32_t line_ = 0;
  std::string function_;
};

}  // namespace logger
