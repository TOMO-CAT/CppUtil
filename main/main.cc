#include <chrono>
#include <filesystem>
#include <thread>
#include <vector>

#include "logger/log.h"
#include "logger/logger.h"
#include "util/time/timestamp.h"

// int main() {
//   CHECK(false) << "it should be true";
//   CHECK_NOTNULL(nullptr);
//   CHECK_EQ(3, 4) << "3 should be equal to 4";
// }

const int THREAD_NUMBER = 10;  // 线程数
const int RUNNING_HOURS = 48;  // 运行小时数

void ThreadRoutine(int idx) {
  LogInfo("thread [%d] start", idx);
  uint64_t t_start = util::TimestampSec();
  while (true) {
    if (util::TimestampSec() - t_start >= RUNNING_HOURS * 3600) {
      break;
    }
    LogInfo("thread [%ld] working...", idx);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  LogInfo("thread [%ld] done", idx);
}

int main() {
  if (!logger::Logger::Instance()->Init("/root/github/CppUtil/logger/test/conf/logger.conf")) {
    LOG_ERROR << "Init logger fail, print to console";
  }

  LOG_INFO << "Init logger successfully!";
  std::vector<std::thread> threads;
  for (size_t i = 0; i < THREAD_NUMBER; i++) {
    threads.push_back(std::thread(ThreadRoutine, i));
  }
  for (auto& t : threads) {
    t.join();
  }

  LOG_INFO << "done and quit!";
  return 0;
}
