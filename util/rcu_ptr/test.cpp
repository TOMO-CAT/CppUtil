#include <sys/time.h>

#include <thread>
#include <vector>

#include "rcu_ptr.h"

inline uint64_t timestamp_us() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 * 1000 + time.tv_usec;
}

inline int rand_int(int max) {
    static unsigned int seed = 1234;
    return rand_r(&seed) % max;
}

// g++ -g test.cpp -o test -pthread -std=c++11
int main() {
    const int READ_THREAD_COUNT = 20;
    const int UPDATE_THREAD_COUNT = 2;
    const int RESET_THREAD_COUNT = 2;
    const int READ_QPS = 10000;
    const int UPDATE_QPS = 20;
    const int RESET_QPS = 20;

    util::rcu_ptr<std::vector<int>> rp(std::make_shared<std::vector<int>>());

    std::vector<std::thread> threads;
    for (int i = 0; i < READ_THREAD_COUNT; i++) {
        threads.push_back(std::thread([&]() {
            uint64_t t_start_us;
            int cnt;
            while (1) {
                t_start_us = timestamp_us();
                cnt = rp.Load()->size();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 / READ_QPS * READ_THREAD_COUNT));
                if (rand_int(10 * READ_QPS) == 0) {
                    printf("[read]cnt:%d cost:%ldus\n", cnt, timestamp_us() - t_start_us);
                }
            }
        }));
    }
    for (int i = 0; i < UPDATE_THREAD_COUNT; i++) {
        threads.push_back(std::thread([&]() {
            uint64_t t_start_us;
            while (1) {
                t_start_us = timestamp_us();
                rp.Update([](std::vector<int>* data) {
                    for (int i = 0; i < 10; i++) {
                        data->push_back(rand_int(100000));
                    }
                });
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 / UPDATE_QPS * UPDATE_THREAD_COUNT));
                if (rand_int(10 * UPDATE_QPS) == 0) {
                    printf("[write]cost:%ldus\n", timestamp_us() - t_start_us);
                }
            }
        }));
    }
    for (int i = 0; i < RESET_QPS; i++) {
        threads.push_back(std::thread([&]() {
            uint64_t t_start_us;
            while (1) {
                t_start_us = timestamp_us();
                std::shared_ptr<std::vector<int>> new_data_sp = std::make_shared<std::vector<int>>();
                new_data_sp->reserve(100);
                for (int i = 0; i < 100; i++) {
                    new_data_sp->push_back(rand_int(10000));
                }
                rp.Store(std::move(new_data_sp));
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 / RESET_QPS * RESET_THREAD_COUNT));
                if (rand_int(10 * RESET_QPS) == 0) {
                    printf("[write]cost:%ldus\n", timestamp_us() - t_start_us);
                }
            }
        }));
    }

    for (auto&& thread : threads) {
        thread.join();
    }
}
