#pragma once

#include <atomic>

#include "lockfree/ipc_message_queue/shared_memory/shared_memory_node_info.h"

namespace cpputil {
namespace lock_free {

/**
 * @brief 基于共享内存的无锁消息队列, 整个存储在共享内存中
 *
 */
struct SharedMemoryMessageQueue {
  static constexpr uint32_t QUEUE_CAPACITY = 2048;

  // 当前最新消息的 sequence
  std::atomic<uint64_t> producer_sequence_ = {0};
  // 消息队列, 每个元素指向了一个共享内存节点
  SharedMemoryNodeInfo node_info_queue_[QUEUE_CAPACITY];
  // 消息队列上每个消息对应的 sequence
  uint64_t msg_sequence_[QUEUE_CAPACITY];
};

}  // namespace lock_free
}  // namespace cpputil
