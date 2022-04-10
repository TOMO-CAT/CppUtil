#pragma once

#include <string>
#include "file_appender.h"

namespace logger {

class Logger : boost::noncopyable {
 public:
    enum class Level {
        DEBUG_LEVEL,
        INFO_LEVEL,
        WARN_LEVEL,
        ERROR_LEVEL,
    };

    /**
     * 初始化, 未初始化或初始化失败时日志会输出到控制台
     */
    bool Init(const std::string& conf_path);
    /**
     * 获取单例
     */
    static Logger* GetInstance() { return instance_; }
    /**
     * 根据日志级别打印日志
     */
    void Log(Level log_level, const char* fmt, ...);

 public:
    static void set_pid(int pid);
    static int get_pid();
    static void set_trace_id(uint64_t trace_id = 0);
    static uint64_t get_trace_id();

 private:
    Logger();
    ~Logger();

 private:
    static std::string gen_timestamp_prefix();

 private:
    static Logger* instance_;
    bool is_console_output_;
    FileAppender* file_appender_;
    Level priority_;
    static __thread uint64_t trace_id_;
    static __thread int pid_;
};

}  // namespace logger

// ======================================对外接口======================================
#define log_debug(fmt, args...) \
do { \
    logger::Logger::GetInstance()->Log(logger::Logger::Level::DEBUG_LEVEL, "[DEBUG][%s:%d][%s]" fmt, \
        __FILE__, __LINE__, __FUNCTION__, ##args); \
} while (0) \

#define log_info(fmt, args...) \
do { \
    logger::Logger::GetInstance()->Log(logger::Logger::Level::INFO_LEVEL, "[INFO ][%s:%d][%s]" fmt, \
        __FILE__, __LINE__, __FUNCTION__, ##args); \
} while (0) \

#define log_warn(fmt, args...) \
do { \
    logger::Logger::GetInstance()->Log(logger::Logger::Level::WARN_LEVEL, "[WARN ][%s:%d][%s]" fmt, \
        __FILE__, __LINE__, __FUNCTION__, ##args); \
} while (0) \

#define log_error(fmt, args...) \
do { \
    logger::Logger::GetInstance()->Log(logger::Logger::Level::ERROR_LEVEL, "[ERROR][%s:%d][%s]" fmt, \
        __FILE__, __LINE__, __FUNCTION__, ##args); \
} while (0) \

#define log_error_t(tag, fmt, args...) \
do { \
    logger::Logger::GetInstance()->Log(logger::Logger::Level::ERROR_LEVEL, "[ERROR][%s:%d][%s][tag=%s]" fmt, \
        __FILE__, __LINE__, __FUNCTION__, tag, ##args); \
} while (0) \

