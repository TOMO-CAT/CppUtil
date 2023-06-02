#include "lockfree/ipc_message_queue/ipc_message_queue.h"

#include <thread>
#include <vector>

#include "gtest/gtest.h"

namespace cpputil {

namespace lock_free {

// 测试并发获取 SharedMemory
TEST(IpcMessageQueueTest, concurrency_get_shared_memory) {
  constexpr uint32_t kThreadCnt = 10;
  constexpr uint32_t kSharedMemoryCount = 100;
  constexpr uint32_t kLoopCount = 1000;

  std::vector<std::thread> thread_list;
  IpcMessageQueue queue;

  for (uint32_t i = 0; i < kThreadCnt; ++i) {
    thread_list.emplace_back(std::thread([]() {
      for (uint32_t k = 0; k < kLoopCount; ++k) {
        for (uint32_t j = 0; j < kSharedMemoryCount; ++j) {
          std::string shared_memory_key = "concurrency_get_shared_memory-" + std::to_string(j);
          IpcMessageQueue::SharedMemory(shared_memory_key);
        }
      }
    }));
  }

  for (auto&& thread : thread_list) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

}  // namespace lock_free
}  // namespace cpputil
