#pragma once

#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/filesystem/file_status.hpp"
#include "boost/interprocess/mapped_region.hpp"
#include "boost/interprocess/shared_memory_object.hpp"
#include "boost/interprocess/sync/file_lock.hpp"
#include "boost/interprocess/sync/sharable_lock.hpp"
#include "lockfree/ipc_message_queue/shared_memory/shared_memory.h"
// NOLINT
#include "boost/interprocess/detail/file_wrapper.hpp"
#include "lockfree/ipc_message_queue/message/message.h"

// to move to *.cpp file
#include "logger/log.h"

namespace cpputil {
namespace lock_free {

// TODO(cc):
// 1. 每块共享内存 (shared_memory_key) 在 producer 退出时可以不用删除
// 2. 研究一下共享内存对象池

/**
 * @brief 单例模式, 维护本进程的共享内存, 可以写入任意消息
 *
 */

/**
 * @brief TODO: 每个生产者有自己的共享内存, 独占式的, 通过 shared_memory_key 区分开
 *
 */
class ReadWriteSharedMemory final : public SharedMemory {
 public:
  /**
   * @brief 构造函数
   *
   * @param shared_memory_key 共享内存 key, 相同的 key 读写同一块共享内存
   * @param shared_memory_size_mb
   */
  ReadWriteSharedMemory(const std::string& shared_memory_key, const uint32_t shared_memory_size_mb)
      : SharedMemory(false, shared_memory_key), shared_memory_size_mb_(shared_memory_size_mb) {
    CHECK_GT(shared_memory_size_mb_, 0);
    file_lock_path_ = "./ring_buffer_shared_memory/" + shared_memory_key_ + ".lock";
    OpenSharedMemory();
  }

 public:
  /**
   * @brief 共享内存环形数组单个节点
   *
   */
  struct Node {
    uint64_t offset = 0;
    uint64_t length = 0;
    uint64_t expired_timestamp = 0;
  };

  /**
   * @brief 共享内存元信息, 存储在
   *
   */

 public:
  bool Write(uint8_t* const data, const uint32_t data_len, NodeInfo* const node_info) override {
    return true;
  }

  bool Read(const NodeInfo& node_info, std::vector<uint8_t>* const data) override {
    return true;
  }

  bool Read(const NodeInfo& node_info, std::string* const data) override {
    return true;
  }

 private:
  /**
   * @brief 打开并映射共享内存空间
   *
   * @return true
   * @return false
   */
  void OpenSharedMemory() {
    std::int64_t desired_size_bytes = shared_memory_size_mb_ << 20;  // * 1024 * 1024

    // 加文件锁避免并发读写风险 (file_lock 析构时会自动释放锁)
    LOG_INFO << "Trying to open shared memory with [non-readonly] mode";
    ::boost::interprocess::file_lock file_lock(file_lock_path_.c_str());
    LOG_INFO << "Trying to lock [" << file_lock_path_ << "] to creating or resize shared memory";
    file_lock.lock();
    LOG_INFO << "Lock [" << file_lock_path_ << "] successfully";

    // 尝试打开或者创建共享内存
    try {
      shared_memory_ = std::make_unique<::boost::interprocess::shared_memory_object>(
          ::boost::interprocess::open_or_create, shared_memory_key_.c_str(), ::boost::interprocess::read_write);
    } catch (boost::interprocess::interprocess_exception& e) {
      LOG_FATAL << e.what();
    }

    // size 非 0 且不一致时报错退出防止影响别人已经申请好的共享内存
    int64_t current_size_bytes = SharedMemoryByteSize();
    if (current_size_bytes != 0 && current_size_bytes != desired_size_bytes) {
      LOG_FATAL << "Unexpected shared memory size [" << current_size_bytes << "] for desired size ["
                << desired_size_bytes << "]";
    }

    // size 为 0 时分配内存
    if (current_size_bytes == 0) {
      LOG_INFO << "Resize shared memory from [" << current_size_bytes << "] to [" << desired_size_bytes << "]";
      shared_memory_->truncate(desired_size_bytes);
    }

    // 建立映射并释放文件锁
    current_size_bytes = SharedMemoryByteSize();
    CHECK_EQ(current_size_bytes, desired_size_bytes);
    LOG_INFO << "Open shared memory [" << shared_memory_key_ << "] with size [" << current_size_bytes
             << "] successfully!";
    mapped_region_ =
        std::make_unique<boost::interprocess::mapped_region>(*shared_memory_, ::boost::interprocess::read_write);
    LOG_INFO << "Trying to unlock [" << file_lock_path_ << "]";
    file_lock.unlock();
    LOG_INFO << "Unlock [" << file_lock_path_ << "] successfully";
  }

 private:
  std::string file_lock_path_;          // 文件锁, 避免多进程并发修改共享内存
  uint32_t shared_memory_size_mb_ = 0;  // 共享内存大小 (单位 MB)
  std::queue<Node> nodes_;              // 分配的内存节点

  uint64_t total_size_bytes_ = 0;  // 总的共享内存大小
  uint64_t free_size_bytes_ = 0;   // 剩余可用内存大小
  uint64_t free_offset_ = 0;       // 当前剩余内存的指针
};

}  // namespace lock_free
}  // namespace cpputil
