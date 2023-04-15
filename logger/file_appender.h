#pragma once

#include <fstream>
#include <string>

#include "boost/noncopyable.hpp"

#define FILE_APPENDER_BUFF_SIZE 4096

namespace logger {

class FileAppender : boost::noncopyable {
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
   * @return RetCode 返回RetCode::OK表示成功, 其他均表示失败
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
  static int64_t gen_now_hour_suffix();
  static int64_t gen_hour_suffix(const struct timeval* tv);
  void cut_if_need();
  void delete_overdue_file(const struct timeval* tv);

 private:
  std::fstream file_stream_;
  std::string file_dir_;
  std::string file_name_;
  std::string file_path_;
  int retain_hours_;
  int64_t last_hour_suffix_;
  pthread_mutex_t write_mutex_;
  static __thread char buffer_[FILE_APPENDER_BUFF_SIZE];
};

}  // namespace logger
