#include "logger/log_capture.h"

#include <sstream>

namespace logger {

LogCapture::LogCapture(const Logger::Level level, const std::string& file, const uint32_t line,
                       const std::string& function, const std::string& check_expression)
    : level_(level), file_(file), line_(line), function_(function), check_expression_(check_expression) {
}

std::ostringstream& LogCapture::stream() {
  return sstream_;
}

LogCapture::~LogCapture() {
  std::string to_print;

  if (level_ == Logger::Level::FATAL_LEVEL) {
    std::ostringstream oss;
    if (!check_expression_.empty()) {
      oss << "\n\tCHECK(" + check_expression_ + ") fail.";
    }
    oss << "\n\tFatal Message: \"" << sstream_.str() << "\"";
    to_print = oss.str();
  } else {
    to_print = sstream_.str();
  }

  Logger::Instance()->Log(level_, "[%s:%d][%s] %s", file_.c_str(), line_, function_.c_str(), to_print.c_str());
}

}  // namespace logger
