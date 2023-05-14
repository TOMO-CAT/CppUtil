#pragma once

#include <atomic>
#include <string>

#include "logger/file_appender.h"

namespace logger {

class Logger {
 public:
  enum class Level {
    DEBUG_LEVEL,
    INFO_LEVEL,
    WARN_LEVEL,
    ERROR_LEVEL,
    FATAL_LEVEL,
  };

  /**
   * 初始化, 未初始化或初始化失败时日志会输出到控制台
   */
  bool Init(const std::string& conf_path);
  /**
   * 获取单例
   */
  static Logger* Instance() {
    return instance_;
  }
  /**
   * 根据日志级别打印日志
   */
  void Log(Level log_level, const char* fmt, ...);

 public:
  // static void set_trace_id(uint64_t trace_id = 0);
  // static uint64_t trace_id();
  static void set_trace_id(const uint64_t trace_id = 0);
  static uint64_t trace_id();

 private:
  Logger();
  ~Logger();

 private:
  static std::string GenLogPrefix();

 private:
  void Backtrace(const uint32_t skip_frames = 1);

 private:
  static Logger* instance_;
  bool is_console_output_ = true;
  FileAppender* file_appender_ = nullptr;
  FileAppender* crash_file_appender_ = nullptr;
  Level priority_ = Level::DEBUG_LEVEL;
  std::atomic<bool> receive_fatal_ = {false};

 private:
  DISALLOW_COPY_AND_ASSIGN(Logger);
};

}  // namespace logger
