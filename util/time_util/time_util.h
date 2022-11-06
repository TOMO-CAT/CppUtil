#pragma once

#include <sys/time.h>
#include <cstdint>
#include <ctime>

namespace util {

/**
 * 返回时间戳， 单位: 秒
 */
uint64_t timestamp_sec() {
    struct timeval tv;
    ::gettimeofday(&tv, nullptr);
    return tv.tv_sec;
}

}  // namespace util