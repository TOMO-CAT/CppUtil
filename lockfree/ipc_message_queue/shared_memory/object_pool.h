#pragma once

#include <atomic>
#include <cstdint>

namespace shared_memory {

/**
 * @brief 基于共享内存的跨进程对象池
 *
 * @note 1. 基于环形数组实现 queue, 方便控制最大内存
 *
 *       2. 基于
 *       3. 加文件锁, 检查到不存在的负责创建
 */
struct ObjectPool {
  static constexpr uint32_t OBJECT_POOL_CAPACITY = 2048;

  struct Node {
    uint64_t offset = 0;
    uint64_t length = 0;
    uint64_t expired_timestamp = 0;
  };

  Node nodes[OBJECT_POOL_CAPACITY];
  std::atomic<uint64_t> front_ = {0};
  std::atomic<uint64_t> rear_ = {0};
  std::atomic<uint64_t> size_t = {0};
};

}  // namespace shared_memory
