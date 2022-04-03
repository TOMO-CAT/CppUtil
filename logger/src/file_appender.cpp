#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include "file_appender.h"
#include "util/macro_util.h"

namespace logger {

__thread char FileAppender::buffer_[FILE_APPENDER_BUFF_SIZE];

FileAppender::FileAppender(std::string dir, std::string file_name, int retain_hours) :
    file_dir_(dir), file_name_(file_name), retain_hours_(retain_hours) {
        if (file_dir_.empty()) {
            file_dir_ = ".";
        }
        file_path_ = file_dir_ + "/" + file_name_;
}

FileAppender::~FileAppender() {
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
}

bool FileAppender::Init() {
    int ret = mkdir(file_dir_.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret != 0 && errno != EEXIST) {
        printf2console("mkdir fail, dir:%s err:%s", file_dir_.c_str(), strerror(errno));
        return false;
    }
    file_stream_.open(file_path_.c_str(), std::fstream::out | std::fstream::app);
    last_hour_suffix_ = gen_now_hour_suffix();
    pthread_mutex_init(&write_mutex_, nullptr);
    return true;
}

void FileAppender::Write(const char* fmt, va_list args) {
    cut_if_need();
    pthread_mutex_lock(&write_mutex_);
    if (file_stream_.is_open()) {
        vsnprintf(buffer_, sizeof(buffer_), fmt, args);
        file_stream_ << buffer_ << "\n";
        file_stream_.flush();
    }
    pthread_mutex_unlock(&write_mutex_);
}

void FileAppender::Write(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Write(fmt, args);
    va_end(args);
}

/**
 * 生成当前小时的文件suffix, 格式yyyymmddhh
 * eg: 2022040214
 */
int64_t FileAppender::gen_now_hour_suffix() {
    struct timeval now;
    ::gettimeofday(&now, nullptr);
    return gen_hour_suffix(&now);
}

int64_t FileAppender::gen_hour_suffix(const struct timeval* tv) {
    struct tm tm_val;
    ::localtime_r(&tv->tv_sec, &tm_val);
    return tm_val.tm_hour + tm_val.tm_mday * 100 + (tm_val.tm_mon + 1) * 10000 + (tm_val.tm_year + 1900) * 1000000;
}

void FileAppender::cut_if_need() {
    struct timeval now;
    ::gettimeofday(&now, nullptr);

    int64_t now_hour_suffix = gen_hour_suffix(&now);
    if (now_hour_suffix > last_hour_suffix_) {
        pthread_mutex_lock(&write_mutex_);
        if (now_hour_suffix > last_hour_suffix_) {
            std::string new_file_path = file_path_ + "." + std::to_string(last_hour_suffix_);  // eg: logger.log.yyyymmddhh
            int ret = rename(file_path_.c_str(), new_file_path.c_str());
            if (ret != 0) {
                printf2console("rename fail, old_file:%s new_file:%s err:%s", file_path_.c_str(), new_file_path.c_str(), strerror(errno));
            }
            file_stream_.close();
            file_stream_.open(file_path_.c_str(), std::fstream::out | std::fstream::app);
            printf2console("cut file, last hour:%d now hour:%d file_path:%s new_file_path:%s",
                last_hour_suffix_, now_hour_suffix, file_path_.c_str(), new_file_path.c_str());
            last_hour_suffix_ = now_hour_suffix;
        }
        pthread_mutex_unlock(&write_mutex_);
        delete_overdue_file(&now);
    }
}

void FileAppender::delete_overdue_file(const struct timeval* tv) {
    if (retain_hours_ <= 0) {
        return;
    }
    struct timeval old_tv;
    old_tv.tv_sec = tv->tv_sec - retain_hours_ * 3600;
    old_tv.tv_usec = tv->tv_usec;

    int64_t old_hour_suffix = gen_hour_suffix(&old_tv);
    std::string old_file_path = file_path_ + "." + std::to_string(old_hour_suffix);
    remove(old_file_path.c_str());
    printf2console("delete old file, file_path:%s", old_file_path.c_str());
}

}  // namespace logger