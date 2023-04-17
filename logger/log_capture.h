#pragma once

#include <sstream>
#include <string>

#include "logger/logger.h"

namespace logger {

class LogCapture {
 public:
  LogCapture(const Logger::Level level, const std::string& file, const uint32_t line, const std::string& function);
  LogCapture(const Logger::Level level, const std::string& file, const uint32_t line, const std::string& function,
             const std::string& check_expression, const std::string& fatal_message);
  ~LogCapture();

 public:
  std::ostringstream& stream();

 private:
  std::ostringstream sstream_;

  Logger::Level level_;
  std::string file_;
  uint32_t line_;
  std::string function_;
  std::string check_expression_;
  std::string fatal_message_;
};

}  // namespace logger