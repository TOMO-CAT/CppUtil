#include "logger/log_capture.h"

namespace logger {

LogCapture::LogCapture(const Logger::Level level, const std::string& file, const uint32_t line,
                       const std::string& function)
    : level_(level), file_(file), line_(line), function_(function) {
}

LogCapture::LogCapture(const Logger::Level level, const std::string& file, const uint32_t line,
                       const std::string& function, const std::string& check_expression,
                       const std::string& fatal_message)
    : level_(level),
      file_(file),
      line_(line),
      function_(function),
      check_expression_(check_expression),
      fatal_message_(fatal_message) {
}

std::ostringstream& LogCapture::stream() {
  return sstream_;
}

LogCapture::~LogCapture() {
  std::string to_print = sstream_.str();

  if (level_ == Logger::Level::FATAL_LEVEL) {
    if (!check_expression_.empty()) {
      sstream_ << "\n\tCHECK(" + check_expression_ + ") fail.";
    }
    if (!fatal_message_.empty()) {
      sstream_ << "\n\tFatal Message: \"" + msg + "\"";
    }
  }
  Logger::Instance()->Log(level_, "[%s:%d][%s] %s", file_.c_str(), line_, function_.c_str(), sstream_.str().c_str());
}

}  // namespace logger
