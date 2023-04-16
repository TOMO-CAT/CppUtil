#pragma once

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

#define __LOGGER_LOG_KV__(log_level, prefix) ::logger::LoggerKV(log_level, __FILE__, __LINE__, __FUNCTION__, prefix)

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