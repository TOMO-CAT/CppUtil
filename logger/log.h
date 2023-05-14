#pragma once

#include <string>

#include "logger/log_capture.h"
#include "logger/log_kv.h"
#include "logger/logger.h"

namespace logger {

// ==================================================== 辅助宏定义 ====================================================
#define __LOGGER_LOG__(log_level, fmt, args...)                                                                 \
  do {                                                                                                          \
    ::logger::Logger::Instance()->Log(log_level, "[%s:%d][%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##args); \
  } while (0)

#define __LOGGER_LOG_WITH_TAG__(log_level, tag, fmt, args...)                                                       \
  do {                                                                                                              \
    ::logger::Logger::Instance()->Log(log_level, "[%s:%d][%s][tag=%s] " fmt, __FILE__, __LINE__, __FUNCTION__, tag, \
                                      ##args);                                                                      \
  } while (0)

#define __LOGGER_LOG_CAPTURE__(log_level) ::logger::LogCapture(log_level, __FILE__, __LINE__, __FUNCTION__).stream()

#define __LOGGER_LOG_CAPTURE_CHECK__(log_level, check_expression) \
  ::logger::LogCapture(log_level, __FILE__, __LINE__, __FUNCTION__, check_expression).stream()

#define __LOGGER_LOG_KV__(log_level, prefix) ::logger::LoggerKV(log_level, __FILE__, __LINE__, __FUNCTION__, prefix)

#define __LOG_EVERY_N__(log_level, N)   \
  static std::atomic<uint32_t> cnt = 0; \
  ++cnt;                                \
  if (cnt > N) {                        \
    cnt -= N;                           \
  }                                     \
  if (cnt == 1) __LOGGER_LOG_CAPTURE__(log_level)

#define __LOG_FIRST_N__(log_level, N)   \
  static std::atomic<uint32_t> cnt = 0; \
  ++cnt;                                \
  if (cnt <= N) __LOGGER_LOG_CAPTURE__(log_level)

// ===================================================== 对外接口 =====================================================

// 格式化日志
#define LogDebug(fmt, args...) __LOGGER_LOG__(::logger::Logger::Level::DEBUG_LEVEL, fmt, ##args)
#define LogInfo(fmt, args...) __LOGGER_LOG__(::logger::Logger::Level::INFO_LEVEL, fmt, ##args)
#define LogWarn(fmt, args...) __LOGGER_LOG__(::logger::Logger::Level::WARN_LEVEL, fmt, ##args)
#define LogError(fmt, args...) __LOGGER_LOG__(::logger::Logger::Level::ERROR_LEVEL, fmt, ##args)
#define LogFatal(fmt, args...) __LOGGER_LOG__(::logger::Logger::Level::FATAL_LEVEL, fmt, ##args)

#define LogErrorWithTag(tag, fmt, args...) \
  __LOGGER_LOG_WITH_TAG__(::logger::Logger::Level::ERROR_LEVEL, tag, fmt, ##args)

// 流式日志
#define LOG_DEBUG __LOGGER_LOG_CAPTURE__(::logger::Logger::Level::DEBUG_LEVEL)
#define LOG_INFO __LOGGER_LOG_CAPTURE__(::logger::Logger::Level::INFO_LEVEL)
#define LOG_WARN __LOGGER_LOG_CAPTURE__(::logger::Logger::Level::WARN_LEVEL)
#define LOG_ERROR __LOGGER_LOG_CAPTURE__(::logger::Logger::Level::ERROR_LEVEL)
#define LOG_FATAL __LOGGER_LOG_CAPTURE__(::logger::Logger::Level::FATAL_LEVEL)

// KV 日志
#define LogDebugKV(prefix) __LOGGER_LOG_KV__(::logger::Logger::Level::DEBUG_LEVEL, prefix)
#define LogInfoKV(prefix) __LOGGER_LOG_KV__(::logger::Logger::Level::INFO_LEVEL, prefix)
#define LogWarnKV(prefix) __LOGGER_LOG_KV__(::logger::Logger::Level::WARN_LEVEL, prefix)
#define LogErrorKV(prefix) __LOGGER_LOG_KV__(::logger::Logger::Level::ERROR_LEVEL, prefix)
#define LogFatalKV(prefix) __LOGGER_LOG_KV__(::logger::Logger::Level::FATAL_LEVEL, prefix)

// 每 N 次打印一条日志
#define LOG_DEBUG_EVERY(N) __LOG_EVERY_N__(::logger::Logger::Level::DEBUG_LEVEL, N)
#define LOG_INFO_EVERY(N) __LOG_EVERY_N__(::logger::Logger::Level::INFO_LEVEL, N)
#define LOG_WARN_EVERY(N) __LOG_EVERY_N__(::logger::Logger::Level::WARN_LEVEL, N)
#define LOG_ERROR_EVERY(N) __LOG_EVERY_N__(::logger::Logger::Level::ERROR_LEVEL, N)

// 打印前 N 条日志
#define LOG_DEBUG_FIRST_N(N) __LOG_FIRST_N__(::logger::Logger::Level::DEBUG_LEVEL, N)
#define LOG_INFO_FIRST_N(N) __LOG_FIRST_N__(::logger::Logger::Level::INFO_LEVEL, N)
#define LOG_WARN_FIRST_N(N) __LOG_FIRST_N__(::logger::Logger::Level::WARN_LEVEL, N)
#define LOG_ERROR_FIRST_N(N) __LOG_FIRST_N__(::logger::Logger::Level::ERROR_LEVEL, N)

// 断言
#define CHECK(expression) \
  if ((expression) == false) __LOGGER_LOG_CAPTURE_CHECK__(::logger::Logger::Level::FATAL_LEVEL, #expression)

#define CHECK_NOTNULL(expression) \
  if ((expression) == nullptr)    \
  __LOGGER_LOG_CAPTURE_CHECK__(::logger::Logger::Level::FATAL_LEVEL, #expression + std::string(" != nullptr"))

#define CHECK_LT(left, right) \
  if (left >= right)          \
  __LOGGER_LOG_CAPTURE_CHECK__(::logger::Logger::Level::FATAL_LEVEL, #left + std::string(" < ") + #right)

#define CHECK_LE(left, right) \
  if (left > right)           \
  __LOGGER_LOG_CAPTURE_CHECK__(::logger::Logger::Level::FATAL_LEVEL, #left + std::string(" <= ") + #right)

#define CHECK_GT(left, right) \
  if (left <= right)          \
  __LOGGER_LOG_CAPTURE_CHECK__(::logger::Logger::Level::FATAL_LEVEL, #left + std::string(" > ") + #right)

#define CHECK_GE(left, right) \
  if (left < right)           \
  __LOGGER_LOG_CAPTURE_CHECK__(::logger::Logger::Level::FATAL_LEVEL, #left + std::string(" >= ") + #right)

#define CHECK_EQ(left, right) \
  if (left != right)          \
  __LOGGER_LOG_CAPTURE_CHECK__(::logger::Logger::Level::FATAL_LEVEL, #left + std::string(" == ") + #right)

#define CHECK_NE(left, right) \
  if (left == right)          \
  __LOGGER_LOG_CAPTURE_CHECK__(::logger::Logger::Level::FATAL_LEVEL, #left + std::string(" != ") + #right)

// 条件日志
#define LOG_INFO_IF(cond) \
  if (cond) LOG_INFO
#define LOG_DEBUG_IF(cond) \
  if (cond) LOG_DEBUG
#define LOG_WARN_IF(cond) \
  if (cond) LOG_WARN
#define LOG_ERROR_IF(cond) \
  if (cond) LOG_ERROR
#define LOG_FATAL_IF(cond) \
  if (cond) LOG_FATAL

/*
#define LogInfo(fmt, args...)                                                                                 \
  do {                                                                                                        \
    logger::Logger::Instance()->Log(logger::Logger::Level::INFO_LEVEL, "[%s:%d][%s]" fmt, __FILE__, __LINE__, \
                                    __FUNCTION__, ##args);                                                    \
  } while (0)

#define LogWarn(fmt, args...)                                                                                 \
  do {                                                                                                        \
    logger::Logger::Instance()->Log(logger::Logger::Level::WARN_LEVEL, "[%s:%d][%s]" fmt, __FILE__, __LINE__, \
                                    __FUNCTION__, ##args);                                                    \
  } while (0)

#define LogError(fmt, args...)                                                                                 \
  do {                                                                                                         \
    logger::Logger::Instance()->Log(logger::Logger::Level::ERROR_LEVEL, "[%s:%d][%s]" fmt, __FILE__, __LINE__, \
                                    __FUNCTION__, ##args);                                                     \
  } while (0)

#define LogErrorWithTag(tag, fmt, args...)                                                                             \
  do {                                                                                                                 \
    logger::Logger::Instance()->Log(logger::Logger::Level::ERROR_LEVEL, "[%s:%d][%s][tag=%s]" fmt, __FILE__, __LINE__, \
                                    __FUNCTION__, tag, ##args);                                                        \
  } while (0)

#define LogFatal(fmt, args...)                                                                                 \
  do {                                                                                                         \
    logger::Logger::Instance()->Log(logger::Logger::Level::FATAL_LEVEL, "[%s:%d][%s]" fmt, __FILE__, __LINE__, \
                                    __FUNCTION__, ##args);                                                     \
  } while (0)
*/

}  // namespace logger
