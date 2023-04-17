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
  if (level_ == Logger::Level::FATAL_LEVEL) {
    if (!check_expression_.empty()) {
      sstream_ << "\n\tCHECK(" + check_expression_ + ") fail.";
    }
  }

  Logger::Instance()->Log(level_, "[%s:%d][%s] %s", file_.c_str(), line_, function_.c_str(), sstream_.str().c_str());
}

}  // namespace logger
