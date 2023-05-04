#include <chrono>
#include <thread>
#include <vector>

#include "logger/file_appender.h"
#include "util/macro_util.h"
#include "util/time/timestamp.h"

using logger::FileAppender;
const int THREAD_NUMBER = 10;  // 线程数
const int RUNNING_HOURS = 24;  // 运行小时数

FileAppender g_file_appender("./log", "TestCrashFileAppender.log", 0, false);

void ThreadRoutine(int idx) {
  g_file_appender.Write("[idx:%d]start!", idx);
  uint64_t t_start = util::TimestampSec();
  while (true) {
    if (util::TimestampSec() - t_start >= RUNNING_HOURS * 3600) {
      break;
    }
    g_file_appender.Write("[idx:%d] working...", idx);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  g_file_appender.Write("[idx:%d]done!", idx);
}

int main() {
  if (!g_file_appender.Init()) {
    printf2console("Error: init fail");
    return -1;
  }
  printf2console("Info: init successfully!");

  g_file_appender.Write("print something");
  g_file_appender.Write("print with fmt, name:%s age:%d weight:%.1f", "tomocat", 24, 56.55);

  std::vector<std::thread> threads;
  for (size_t i = 0; i < THREAD_NUMBER; i++) {
    threads.push_back(std::thread(ThreadRoutine, i));
  }
  for (auto& t : threads) {
    t.join();
  }

  printf2console("Info: done and quit!");
  return 0;
}
