#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "lockfree/ipc_message_queue/shared_memory/ring_buffer_shared_memory.h"
#include "lockfree/ipc_message_queue/shared_memory/shared_memory_message_queue.h"
#include "lockfree/ipc_message_queue/shared_memory/shared_memory_node_info.h"

namespace cpputil {
namespace lock_free {
/**
 * @brief 单例模式, 负责进程间通信, 可以读写多块共享内存 (shared_memory_key 区分)
 *
 */
class IpcMessageQueue {
 public:
  IpcMessageQueue() {
  }
  ~IpcMessageQueue() {
  }

 public:
  struct MessageInfo {
    int32_t process_id = 0;
    uint32_t channel_id = 0;
  };

 public:
  void Read(std::vector<uint8_t>* const message);
  void Write(const std::vector<uint8_t>& message);
  void Write(uint8_t* const data, const uint32_t data_len);

 private:
  static std::unordered_map<std::string, std::shared_ptr<RingBufferSharedMemory>> central_shared_memory_map_;
  static thread_local std::unordered_map<std::string, std::shared_ptr<RingBufferSharedMemory>> local_shared_memory_map;
  static std::mutex central_shared_memory_map_mutex_;

 public:
  /**
   * @brief 无锁获取 shared_memory_key 对应的 RingBufferSharedMemory
   *
   * @param shared_memory_key
   * @return std::shared_ptr<RingBufferSharedMemory>
   */
  static std::shared_ptr<RingBufferSharedMemory> SharedMemory(const std::string& shared_memory_key) {
    // 1. 尝试从 local map 中读取 SharedMemory, 有的话直接返回
    auto iter = local_shared_memory_map.find(shared_memory_key);
    if (iter != local_shared_memory_map.end()) {
      // LOG_DEBUG << "Find shared memory successfully from local map with key [" << shared_memory_key;
      return iter->second;
    }

    LOG_INFO << "Failed to find shared memory with key [" << shared_memory_key
             << "] locally, try to find in central map";

    // 2. 读取不到的话加锁从 central map 中尝试获取
    {
      std::lock_guard<std::mutex> lk(central_shared_memory_map_mutex_);

      // 2.1 central map 中有则直接更新 local map
      auto central_iter = central_shared_memory_map_.find(shared_memory_key);
      if (central_iter != central_shared_memory_map_.end()) {
        local_shared_memory_map[shared_memory_key] = central_iter->second;
        LOG_INFO << "Find shared memory successfully from central map with key [" << shared_memory_key;
        return central_iter->second;
      }

      // 2.2 central map 中没有则 new 一个 RingBufferSharedMemory
      auto shared_memory_sptr = std::make_shared<RingBufferSharedMemory>(shared_memory_key);
      central_shared_memory_map_[shared_memory_key] = shared_memory_sptr;
      local_shared_memory_map[shared_memory_key] = shared_memory_sptr;
      LOG_INFO << "New shared memory successfully with key [" << shared_memory_key;
      return shared_memory_sptr;
    }
  }

 private:
  std::unique_ptr<SharedMemoryMessageQueue> queue_;
  std::atomic<uint64_t> consumer_sequence_ = {0};  // 当前进程的消费 sequence
};

std::unordered_map<std::string, std::shared_ptr<RingBufferSharedMemory>> IpcMessageQueue::central_shared_memory_map_;
thread_local std::unordered_map<std::string, std::shared_ptr<RingBufferSharedMemory>>
    IpcMessageQueue::local_shared_memory_map;
std::mutex IpcMessageQueue::central_shared_memory_map_mutex_;

}  // namespace lock_free
}  // namespace cpputil
