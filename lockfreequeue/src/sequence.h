#pragma once

#include <cstdlib>
#include <atomic>

constexpr int64_t kInitialCursorValue = -1L;

// 并发安全的序号, 管理交换的数据和事件
// 跟踪标识某个特定的事件处理者(RingBuffer/Producer/Consumer)的处理进度
namespace cpputil {

class Sequence {
 public:
    explicit Sequence(int64_t initial_value = kInitialCursorValue) {}

    // TODO(yang): 内存顺序
    // 本线程中,所有后续的读操作必须在本条原子操作完成后执行
    int64_t get_sequence() const {
        return sequence_.load(std::memory_order::memory_order_acquire);
    }

    void set_sequence(int64_t value) {
        sequence_.store(value, std::memory_order::memory_order_release);
    }

    int64_t FetchAdd(int64_t increment) {
        return sequence_.fetch_add(increment, std::memory_order::memory_order_release) + increment;
    }


 private:
    std::atomic<int64_t> sequence_;
};

}  // namespace cpputil
