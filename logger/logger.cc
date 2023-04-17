#include "logger/logger.h"

#include <execinfo.h>
#include <sys/time.h>
#include <uuid/uuid.h>

#include <array>
#include <cstdarg>
#include <memory>
#include <string>
#include <unordered_map>

#include "cpptoml/cpptoml.h"
#include "logger/log.h"
#include "util/config_util/toml_helper.h"
namespace logger {

namespace {
const std::unordered_map<Logger::Level, std::string> kLevel2Description = {
    {Logger::Level::DEBUG_LEVEL, "[DEBUG]"}, {Logger::Level::INFO_LEVEL, "[INFO ]"},
    {Logger::Level::WARN_LEVEL, "[WARN ]"},  {Logger::Level::ERROR_LEVEL, "[ERROR]"},
    {Logger::Level::FATAL_LEVEL, "[FATAL]"},
};
}

Logger* Logger::instance_ = new Logger();
__thread uint64_t Logger::trace_id_ = 0;
__thread int Logger::pid_ = 0;

Logger::Logger() : is_console_output_(true), file_appender_(nullptr), priority_(Level::INFO_LEVEL) {
}

Logger::~Logger() {
  if (file_appender_) {
    delete file_appender_;
  }
}

bool Logger::Init(const std::string& conf_path) {
  set_pid(pid());
  set_trace_id();

  std::shared_ptr<cpptoml::table> g;
  try {
    g = cpptoml::parse_file(conf_path);
  } catch (const cpptoml::parse_exception& e) {
    LogError("parse logger conf fail, path:%s err:%s", conf_path.c_str(), e.what());
    return false;
  }

  int level;
  std::string dir;
  std::string file_name;
  int retain_hours;
  if (util::ParseTomlValue(g, "Level", &level)) {
    if (level >= static_cast<int>(Level::DEBUG_LEVEL) && level <= static_cast<int>(Level::ERROR_LEVEL)) {
      priority_ = Level(level);
    }
  }
  if (!util::ParseTomlValue(g, "Directory", &dir)) {
    dir = ".";
  }
  if (!util::ParseTomlValue(g, "FileName", &file_name)) {
    LogWarn("parse FileName config fail, print to console");
    return false;
  }
  if (!util::ParseTomlValue(g, "RetainHours", &retain_hours)) {
    retain_hours = 0;  // don't delete overdue log file
  }
  file_appender_ = new FileAppender(dir, file_name, retain_hours);
  if (!file_appender_->Init()) {
    return false;
  }
  is_console_output_ = false;
  return true;
}

void Logger::Log(Level log_level, const char* fmt, ...) {
  if (log_level < priority_) {
    return;
  }

  std::string new_fmt = GenLogPrefix() + kLevel2Description.at(log_level) + " " + fmt;
  if (log_level == Level::FATAL_LEVEL) {
    new_fmt += "\n\tExiting due to FATAL log";
    new_fmt += "\n\tCall Stack:";
  }

  va_list args;
  va_start(args, fmt);

  // ERROR 及 FATAL 日志输出到控制台
  if (is_console_output_ || log_level >= Level::ERROR_LEVEL) {
// https://stackoverflow.com/questions/36120717/correcting-format-string-is-not-a-string-literal-warning
#if defined(__has_warning)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    vprintf((new_fmt + "\n").c_str(), args);
#if defined(__has_warning)
#pragma clang diagnostic pop
#endif
  } else {
    file_appender_->Write(new_fmt.c_str(), args);
  }
  va_end(args);

  if (log_level == Level::FATAL_LEVEL) {
    // 不可重入, 防止打印多个 FATAL 日志
    if (!receive_fatal_.exchange(true)) {
      Backtrace();
      std::abort();
    }
  }
}

void Logger::Backtrace(const uint32_t skip_frames) {
  constexpr uint32_t kMaxFrames = 128;
  std::array<void*, kMaxFrames> call_stacks;

  int32_t frame_size = ::backtrace(call_stacks.data(), kMaxFrames);
  std::shared_ptr<char*> symbols(::backtrace_symbols(call_stacks.data(), frame_size), std::free);
  if (symbols) {
    for (int i = skip_frames; i < frame_size; ++i) {
      printf("%s\n", (std::string("\t\t") + symbols.get()[i]).c_str());
      file_appender_->Write((std::string("\t\t") + symbols.get()[i]).c_str());
    }
  }
  if (frame_size == kMaxFrames) {
    printf("\t\t[truncated]\n");
    file_appender_->Write("\t\t[truncated]");
  }
  file_appender_->Write("\n\n");
}

std::string Logger::GenLogPrefix() {
  struct timeval now;
  ::gettimeofday(&now, nullptr);
  struct tm tm_now;
  ::localtime_r(&now.tv_sec, &tm_now);
  char time_str[100];
  snprintf(time_str, sizeof(time_str), "[%04d-%02d-%02d %02d:%02d:%02d.%06ld][%d:%lx]", tm_now.tm_year + 1900,
           tm_now.tm_mon + 1, tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec, now.tv_usec, pid_,
           trace_id_);
  return time_str;
}

void Logger::set_pid(int pid) {
  pid_ = pid;
}

int Logger::pid() {
  return pid_;
}

void Logger::set_trace_id(uint64_t trace_id) {
  if (trace_id == 0) {
    uuid_t uuid;
    uuid_generate(uuid);
    // 将 uuid 解析成 uint64_t 数组并取第一个元素作为 trace_id
    uint64_t* trace_id_list = reinterpret_cast<uint64_t*>(uuid);
    trace_id_ = trace_id_list[0];
  } else {
    trace_id_ = trace_id;
  }
}

uint64_t Logger::trace_id() {
  return trace_id_;
}

}  // namespace logger