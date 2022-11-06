#include <iostream>
#include <map>
#include <random>
#include <string>
#include <thread>

#include "double_buffer.h"

int rand_int(int max) {
    static unsigned int seed = 1234;
    return rand_r(&seed) % max;
}

void reader(DoubleBuffer<std::map<int, std::string>>& db) {
    while (1) {
        int not_empty_cnt = 0;
        auto data_sptr = db.Load();
        for (int i = 0; i < 100; i++) {
            auto iter = data_sptr->find(rand_int(10000));
            if (iter != data_sptr->end()) {
                not_empty_cnt++;
            }
        }
        // print out one percent of the log
        if (rand_int(100) == 0) {
            printf("[read]not empty key cnt:%d map len:%ld\n", not_empty_cnt, data_sptr->size());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void writer(DoubleBuffer<std::map<int, std::string>>& db) {
    while (1) {
        // you can use lambda expression to pass any parameter
        int count = 0;
        db.Update([&count](std::map<int, std::string>& data) {
            for (int i = 100; i < 200; i++) {
                data[rand_int(10000)] = std::to_string(i * i);
                count++;
            }
        });
        printf("[write]write count:%d\n", count);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// g++ -g test.cpp -o test -pthread
int main() {
    std::map<int, std::string> data;
    for (int i = 0; i < 100; i++) {
        data[i] = std::to_string(i * i);
    }
    DoubleBuffer<std::map<int, std::string>> double_buffer(data);

    std::vector<std::thread> threads;
    // 20 reader thread
    for (int i = 0; i < 20; i++) {
        threads.push_back(std::thread(reader, std::ref(double_buffer)));
    }

    // 1 writer thread
    threads.push_back(std::thread(writer, std::ref(double_buffer)));

    for (auto&& thread : threads) {
        thread.join();
    }

    return 0;
}