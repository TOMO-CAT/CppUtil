#pragma once

#include <sys/time.h>

#include <cstdint>
#include <ctime>

namespace util {

/**
 * 返回时间戳， 单位: 秒
 */
inline uint64_t TimestampSec() {
  struct timeval tv;
  ::gettimeofday(&tv, nullptr);
  return tv.tv_sec;
}

/**
 * 返回时间戳， 单位: 微秒
 */
inline uint64_t TimestampMicroSec() {
  struct timeval time;
  gettimeofday(&time, NULL);
  return time.tv_sec * 1000 * 1000 + time.tv_usec;
}

}  // namespace util
