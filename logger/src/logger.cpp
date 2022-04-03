#include <uuid/uuid.h>
#include <sys/time.h>
#include <memory>
#include <cstdarg>
#include "logger.h"
#include "cpptoml.h"
#include "util/config_util/toml_helper.h"

namespace logger {

Logger* Logger::instance_ = new Logger();
__thread uint64_t Logger::trace_id_ = 0;
__thread int Logger::pid_ = 0;

Logger::Logger() : is_console_output_(true), file_appender_(nullptr), priority_(Level::INFO_LEVEL) {}

Logger::~Logger() {
    if (file_appender_) {
        delete file_appender_;
    }
}

bool Logger::Init(const std::string& conf_path) {
    set_pid(getpid());
    set_trace_id();

    std::shared_ptr<cpptoml::table> g;
    try {
        g = cpptoml::parse_file(conf_path);
    } catch (const cpptoml::parse_exception& e) {
        log_error("parse logger conf fail, path:%s err:%s", conf_path.c_str(), e.what());
        return false;
    }

    int level;
    std::string dir;
    std::string file_name;
    int retain_hours;
    if (util::ParseTomlValue(g, "Level", level)) {
        if (level >= static_cast<int>(Level::DEBUG_LEVEL) && level <= static_cast<int>(Level::ERROR_LEVEL)) {
            priority_ = Level(level);
        }
    }
    if (!util::ParseTomlValue(g, "Directory", dir)) {
        dir = ".";
    }
    if (!util::ParseTomlValue(g, "FileName", file_name)) {
        log_warn("parse FileName config fail, print to console");
        return false;
    }
    if (!util::ParseTomlValue(g, "RetainHours", retain_hours)) {
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

    std::string new_fmt = gen_timestamp_prefix() + fmt;

    va_list args;
    va_start(args, fmt);
    if (is_console_output_) {
        vprintf((new_fmt + "\n").c_str(), args);
    } else {
        file_appender_->Write(new_fmt.c_str(), args);
    }
    va_end(args);
}

std::string Logger::gen_timestamp_prefix() {
    struct timeval now;
    ::gettimeofday(&now, nullptr);
    struct tm tm_now;
    ::localtime_r(&now.tv_sec, &tm_now);
    char time_str[100];
    snprintf(time_str, sizeof(time_str), "[%04d-%02d-%02d %02d:%02d:%02d.%06ld][%d:%lx]",
        tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec, now.tv_usec,
        pid_, trace_id_);
    return time_str;
}

void Logger::set_pid(int pid) {
    pid_ = pid;
}

int Logger::get_pid() {
    return pid_;
}

void Logger::set_trace_id(uint64_t trace_id) {
    if (trace_id == 0) {
        uuid_t uuid;
        uuid_generate(uuid);
        // 将uuid解析成uint64_t数组并取第一个元素作为trace_id
        uint64_t* trace_id_list = reinterpret_cast<uint64_t*>(uuid);
        trace_id_ = trace_id_list[0];
    } else {
        trace_id_ = trace_id;
    }
}

uint64_t Logger::get_trace_id() {
    return trace_id_;
}

}  // namespace logger