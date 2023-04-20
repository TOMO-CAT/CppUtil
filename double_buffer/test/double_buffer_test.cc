#include "double_buffer/double_buffer.h"

#include <sys/time.h>

#include <iostream>
#include <map>
#include <random>
#include <string>
#include <thread>

const int QPS = 10000;
const int READ_THREAD_CNT = 20;
const int UPDATE_THREAD_CNT = 3;
const int RESET_THREAD_CNT = 1;

int rand_int(int max) {
  static unsigned int seed = 1234;
  return rand_r(&seed) % max;
}

uint64_t timestamp_us() {
  struct timeval time;
  gettimeofday(&time, NULL);
  return time.tv_sec * 1000 * 1000 + time.tv_usec;
}

void reader(const util::DoubleBuffer<std::map<int, std::string>>& db) {
  while (1) {
    auto t_start_us = timestamp_us();
    int not_empty_cnt = 0;
    auto data_sptr = db.Load();
    for (int i = 0; i < 100; i++) {
      auto iter = data_sptr->find(rand_int(1000000));
      if (iter != data_sptr->end()) {
        not_empty_cnt++;
      }
    }
    if (rand_int(QPS * 10) == 0) {
      printf("[read]not empty key cnt:%d map len:%ld cost:%ld us\n", not_empty_cnt, data_sptr->size(),
             timestamp_us() - t_start_us);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / QPS * READ_THREAD_CNT));
  }
}

void update_writer(util::DoubleBuffer<std::map<int, std::string>>* const db) {
  while (1) {
    // you can use lambda expression to pass any parameter
    auto t_start_us = timestamp_us();
    db->Update([](std::map<int, std::string>* const data) {
      for (int i = 100; i < 200; i++) {
        (*data)[rand_int(1000000)] = std::to_string(i * i);
      }
    });
    if (rand_int(200) == 0) {
      printf("[update writer]cost %ld us\n", timestamp_us() - t_start_us);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void reset_writer(util::DoubleBuffer<std::map<int, std::string>>* const db) {
  while (1) {
    std::map<int, std::string> data;
    for (int i = 0; i < 1000; i++) {
      data[rand_int(100)] = std::to_string(i);
    }
    auto t_start_us = timestamp_us();
    db->Reset(data);
    printf("[reset writer]cost %ld us\n", timestamp_us() - t_start_us);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

// g++ -g test.cpp -o test -pthread -std=c++11
int main() {
  // initialization
  std::map<int, std::string> data;
  for (int i = 0; i < 1000; i++) {
    data[i] = std::to_string(i * i);
  }
  util::DoubleBuffer<std::map<int, std::string>> double_buffer(std::move(data));
  std::vector<std::thread> threads;

  // reader thread
  for (int i = 0; i < READ_THREAD_CNT; i++) {
    threads.push_back(std::thread(reader, std::ref(double_buffer)));
  }

  // update thread
  for (int i = 0; i < UPDATE_THREAD_CNT; i++) {
    threads.push_back(std::thread(update_writer, &double_buffer));
  }

  // reset thread
  for (int i = 0; i < RESET_THREAD_CNT; i++) {
    threads.push_back(std::thread(reset_writer, &double_buffer));
  }

  for (auto&& thread : threads) {
    thread.join();
  }

  return 0;
}
