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
};

}  // namespace logger