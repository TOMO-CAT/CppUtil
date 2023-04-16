#include "logger/file_appender.h"

#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "util/macro_util.h"

namespace logger {

__thread char FileAppender::buffer_[kFileAppenderBuffSize];

FileAppender::FileAppender(std::string dir, std::string file_name, int retain_hours)
    : file_dir_(dir), file_name_(file_name), retain_hours_(retain_hours) {
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
  last_hour_suffix_ = GenNowHourSuffix();
  pthread_mutex_init(&write_mutex_, nullptr);
  return true;
}

void FileAppender::Write(const char* fmt, va_list args) {
  CutIfNeed();
  pthread_mutex_lock(&write_mutex_);
  if (file_stream_.is_open()) {
// https://stackoverflow.com/questions/36120717/correcting-format-string-is-not-a-string-literal-warning
#if defined(__has_warning)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    vsnprintf(buffer_, sizeof(buffer_), fmt, args);
#if defined(__has_warning)
#pragma clang diagnostic pop
#endif
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
 * 生成当前小时的文件 suffix, 格式 yyyymmddhh
 * eg: 2022040214
 */
int64_t FileAppender::GenNowHourSuffix() {
  struct timeval now;
  ::gettimeofday(&now, nullptr);
  return GenHourSuffix(&now);
}

int64_t FileAppender::GenHourSuffix(const struct timeval* tv) {
  struct tm tm_val;
  ::localtime_r(&tv->tv_sec, &tm_val);
  return tm_val.tm_hour + tm_val.tm_mday * 100 + (tm_val.tm_mon + 1) * 10000 + (tm_val.tm_year + 1900) * 1000000;
}

void FileAppender::CutIfNeed() {
  struct timeval now;
  ::gettimeofday(&now, nullptr);

  int64_t now_hour_suffix = GenHourSuffix(&now);
  if (now_hour_suffix > last_hour_suffix_) {
    pthread_mutex_lock(&write_mutex_);
    if (now_hour_suffix > last_hour_suffix_) {
      std::string new_file_path = file_path_ + "." + std::to_string(last_hour_suffix_);  // eg: logger.log.yyyymmddhh
      int ret = rename(file_path_.c_str(), new_file_path.c_str());
      if (ret != 0) {
        printf2console("rename fail, old_file:%s new_file:%s err:%s", file_path_.c_str(), new_file_path.c_str(),
                       strerror(errno));
      }
      file_stream_.close();
      file_stream_.open(file_path_.c_str(), std::fstream::out | std::fstream::app);
      printf2console("cut file, last hour:%ld now hour:%ld file_path:%s new_file_path:%s", last_hour_suffix_,
                     now_hour_suffix, file_path_.c_str(), new_file_path.c_str());
      last_hour_suffix_ = now_hour_suffix;
    }
    pthread_mutex_unlock(&write_mutex_);
    DeleteOverdueFile(&now);
  }
}

void FileAppender::DeleteOverdueFile(const struct timeval* tv) {
  if (retain_hours_ <= 0) {
    return;
  }
  struct timeval old_tv;
  old_tv.tv_sec = tv->tv_sec - retain_hours_ * 3600;
  old_tv.tv_usec = tv->tv_usec;

  int64_t old_hour_suffix = GenHourSuffix(&old_tv);
  std::string old_file_path = file_path_ + "." + std::to_string(old_hour_suffix);
  remove(old_file_path.c_str());
  printf2console("delete old file, file_path:%s", old_file_path.c_str());
}

}  // namespace logger