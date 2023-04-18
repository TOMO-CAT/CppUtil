#include "logger/log_kv.h"

#include <cstdarg>

namespace logger {

LoggerKV::LoggerKV(const Logger::Level level, const std::string& file, const uint32_t line, const std::string& function,
                   const std::string& prefix)
    : level_(level), file_(file), line_(line), function_(function) {
  sstream_ << prefix;
}

LoggerKV& LoggerKV::LogKVFormat(const char* const format, ...) {
  va_list args;
  va_start(args, format);
  char buff[200];
#if defined(__has_warning)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
  vsnprintf(buff, sizeof(buff), format, args);
#if defined(__has_warning)
#pragma clang diagnostic pop
#endif
  sstream_ << "||" << buff;
  va_end(args);
  return *this;
}

LoggerKV::~LoggerKV() {
  Logger::Instance()->Log(level_, "[%s:%d][%s] %s", file_.c_str(), line_, function_.c_str(), sstream_.str().c_str());
}

}  // namespace logger
