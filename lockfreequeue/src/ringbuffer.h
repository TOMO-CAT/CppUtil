#pragma once

#include <iostream>
#include <array>
#include <boost/noncopyable.hpp>

namespace cpputil {

template <typename T, size_t N = 1024>
class RingBuffer : boost::noncopyable {
 public:
    explicit RingBuffer(const std::array<T, N>& events) : events_(events) {}

    // 令 RingBuffer 的长度为 2 的 N 次幂, 从而利用位运算提高取模效率
    static_assert(((N > 0) && ((N & (~N + 1)) == N)),
                "RingBuffer's size must be a positive power of 2");

    T& operator[](int64_t sequence) {
        return events_[sequence & (N - 1)];
    }

 private:
    std::array<T, N> events_;
};


}  // namespace cpputil