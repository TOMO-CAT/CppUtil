#pragma once

#include <fstream>
#include <string>

#include "util/macro_util.h"

namespace logger {

class FileAppender {
 public:
  /**
   * @brief Construct a new File Appender object
   *
   * @param dir 日志保存路径
   * @param file_name 日志名
   * @param retain_hours 保存小时数
   */
  FileAppender(std::string dir, std::string file_name, int retain_hours);
  ~FileAppender();

 public:
  /**
   * @brief 必要的初始化
   *
   * @return 返回 RetCode::OK 表示成功, 其他均表示失败
   */
  bool Init();
  /**
   * @brief 将格式化字符串写入日志文件
   *
   * @param fmt 带有格式控制符的字符串
   * @param args 参数
   */
  void Write(const char* fmt, va_list args);
  void Write(const char* fmt, ...);

 private:
  static int64_t GenNowHourSuffix();
  static int64_t GenHourSuffix(const struct timeval* tv);
  void CutIfNeed();
  void DeleteOverdueFile(const struct timeval* tv);

 private:
  static constexpr uint32_t kFileAppenderBuffSize = 4096;

 private:
  std::fstream file_stream_;
  std::string file_dir_;
  std::string file_name_;
  std::string file_path_;
  int retain_hours_;
  int64_t last_hour_suffix_;
  pthread_mutex_t write_mutex_;
  static __thread char buffer_[kFileAppenderBuffSize];

  DISALLOW_COPY_AND_ASSIGN(FileAppender);
};

}  // namespace logger
