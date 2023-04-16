#include "logger/log_capture.h"

namespace logger {

LogCapture::LogCapture(const Logger::Level level, const std::string& file, const uint32_t line,
                       const std::string& function)
    : level_(level), file_(file), line_(line), function_(function) {
}

std::ostringstream& LogCapture::stream() {
  return sstream_;
}

LogCapture::~LogCapture() {
  Logger::Instance()->Log(level_, "[%s:%d][%s] %s", file_.c_str(), line_, function_.c_str(), sstream_.str().c_str());
}

}  // namespace logger
