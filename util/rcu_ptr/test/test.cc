#include <sys/time.h>

#include <thread>
#include <vector>

#include "logger/log.h"
#include "util/random.h"
#include "util/rcu_ptr/rcu_ptr.h"
#include "util/time/timestamp.h"

// g++ -g test.cpp -o test -pthread -std=c++11
int main() {
  const int READ_THREAD_COUNT = 20;
  const int UPDATE_THREAD_COUNT = 2;
  const int RESET_THREAD_COUNT = 2;
  const int READ_QPS = 10000;
  const int UPDATE_QPS = 20;
  const int RESET_QPS = 1;

  util::rcu_ptr<std::vector<int>> rp(std::make_shared<std::vector<int>>());

  std::vector<std::thread> threads;
  for (int i = 0; i < READ_THREAD_COUNT; i++) {
    threads.push_back(std::thread([&]() {
      uint64_t t_start_us;
      int cnt;
      while (1) {
        t_start_us = util::TimestampMicroSec();
        cnt = rp.Load()->size();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / READ_QPS * READ_THREAD_COUNT));
        if (util::RandInt(100 * READ_QPS) == 0) {
          LogInfo("[reader] cnt:%d cost:%ld us", cnt, util::TimestampMicroSec() - t_start_us);
        }
      }
    }));
  }
  for (int i = 0; i < UPDATE_THREAD_COUNT; i++) {
    threads.push_back(std::thread([&]() {
      uint64_t t_start_us;
      while (1) {
        t_start_us = util::TimestampMicroSec();
        rp.Update([](std::vector<int>* write_buffer_data) {
          for (int i = 0; i < 10; i++) {
            write_buffer_data->push_back(util::RandInt(100000));
          }
        });
        if (util::RandInt(10 * UPDATE_QPS) == 0) {
          LogInfo("[update writer] cost:%ld us", util::TimestampMicroSec() - t_start_us);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / UPDATE_QPS * UPDATE_THREAD_COUNT));
      }
    }));
  }
  for (int i = 0; i < RESET_THREAD_COUNT; i++) {
    threads.push_back(std::thread([&]() {
      uint64_t t_start_us;
      while (1) {
        std::shared_ptr<std::vector<int>> new_data_sp = std::make_shared<std::vector<int>>();
        new_data_sp->reserve(100);
        for (int i = 0; i < 300; i++) {
          new_data_sp->push_back(util::RandInt(10000));
        }
        t_start_us = util::TimestampMicroSec();
        rp.Store(std::move(new_data_sp));
        if (util::RandInt(10 * RESET_QPS) == 0) {
          LogInfo("[reset writer] cost:%ld us", util::TimestampMicroSec() - t_start_us);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / RESET_QPS * RESET_THREAD_COUNT));
      }
    }));
  }

  for (auto&& thread : threads) {
    thread.join();
  }
}
