/**
 * @file TestFileAppender.cpp
 * @author tomocat (512676561@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-04-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <thread>
#include <vector>
#include "util/macro_util.h"
#include "util/time_util/time_util.h"
#include "file_appender.h"

using logger::FileAppender;
const int THREAD_NUMBER = 10;   // 线程数
const int RUNNING_HOURS = 24;   // 运行小时数
const int RETAIN_HOURS = 6;     // 日志保留小时数

FileAppender g_file_appender("./log", "TestFileAppender.log", RETAIN_HOURS);

void ThreadRoutine(int idx) {
    g_file_appender.Write("[idx:%d]start!", idx);
    uint64_t t_start = util::timestamp_sec();
    while (true) {
        if (util::timestamp_sec() - t_start >= RUNNING_HOURS * 3600) {
            break;
        }
        g_file_appender.Write("[idx:%d]wroking...", idx);
        sleep(1);
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
