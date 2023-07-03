#pragma once

#include <memory>
#include <queue>
#include <string>

#include "boost/filesystem.hpp"
#include "boost/filesystem/file_status.hpp"
#include "boost/interprocess/mapped_region.hpp"
#include "boost/interprocess/shared_memory_object.hpp"
#include "boost/interprocess/sync/file_lock.hpp"
#include "boost/interprocess/sync/sharable_lock.hpp"
// NOLINT
#include "boost/interprocess/detail/file_wrapper.hpp"
#include "lockfree/ipc_message_queue/message/message.h"

// to move to *.cpp file
#include "logger/log.h"

namespace cpputil {
namespace lock_free {

/**
 * @brief 单例模式, 维护本进程的共享内存, 可以写入任意消息
 *
 */

/**
 * @brief TODO: 每个生产者有自己的共享内存, 独占式的, 通过 shared_memory_key 区分开
 *
 */
class RingBufferSharedMemory {
 public:
  /**
   * @brief 构造函数
   *
   * @param shared_memory_key 共享内存 key, 相同的 key 读写同一块共享内存
   * @param shared_memory_size_mb
   */
  RingBufferSharedMemory(const std::string& shared_memory_key, const uint32_t shared_memory_size_mb)
      : shared_memory_key_(shared_memory_key), shared_memory_size_mb_(shared_memory_size_mb), is_readonly_(false) {
    CHECK(!shared_memory_key_.empty());
    CHECK_GT(shared_memory_size_mb_, 0);
    file_lock_path_ = "./ring_buffer_shared_memory/" + shared_memory_key_ + ".lock";
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
  bool Write(uint8_t* const data, const uint32_t data_len);

 private:
  /**
   * @brief 打开并映射共享内存空间
   *
   * @return true
   * @return false
   */
  bool OpenSharedMemory() {
    std::int64_t desired_size_bytes = shared_memory_size_mb_ << 20;  // * 1024 * 1024

    // 1. 只读模式
    if (is_readonly_) {
      LOG_INFO << "Trying to open shared memory with [readonly] mode";
      // 1.1 共享内存文件尚未创建, 延迟访问
      try {
        shared_memory_ = std::make_unique<::boost::interprocess::shared_memory_object>(
            ::boost::interprocess::open_only, shared_memory_key_.c_str(), ::boost::interprocess::read_only);
      } catch (boost::interprocess::interprocess_exception& e) {
        LOG_WARN << "Failed to open shared memory with error: " << e.what();
        return false;
      }
      CHECK_NOTNULL(shared_memory_);

      // 1.2 虽然有共享内存, 但是 size 为 0, 延迟访问
      int64_t current_size_bytes = SharedMemorySize();
      if (current_size_bytes == 0) {
      }

      // 1.2 虽然有共享内存, 但是 size 不一致, 延迟访问
      int64_t current_size_bytes = SharedMemorySize();
      if (current_size_bytes != desired_size_bytes) {
        LOG_WARN << "Unexpected shared memory size [" << current_size_bytes << "] for desired size ["
                 << desired_size_bytes << "], waiting for subsequent re-creation of appropriately sized shared memory";
        shared_memory_ = nullptr;
        return false;
      }

      // 1.3 有共享内存且内存大小一致
      LOG_INFO << "Open shared memory [" << shared_memory_key_ << "] with size [" << current_size_bytes
               << "] successfully!";
      mapped_region_ =
          std::make_unique<boost::interprocess::mapped_region>(*shared_memory_, ::boost::interprocess::read_only);
      is_available_ = true;
      return true;
    } else {
      // 2. 读写模式 (加文件锁避免并发读写风险, 析构时会释放锁)
      LOG_INFO << "Trying to open shared memory with [non-readonly] mode";
      ::boost::interprocess::file_lock file_lock(file_lock_path_.c_str());
      LOG_INFO << "Trying to lock [" << file_lock_path_ << "] to creating or resize shared memory";
      file_lock.lock();
      LOG_INFO << "Lock [" << file_lock_path_ << "] successfully";

      // 2.1 尝试打开或者创建共享内存
      try {
        shared_memory_ = std::make_unique<::boost::interprocess::shared_memory_object>(
            ::boost::interprocess::open_or_create, shared_memory_key_.c_str(), ::boost::interprocess::read_write);
      } catch (boost::interprocess::interprocess_exception& e) {
        LOG_FATAL << e.what();
      }

      int64_t current_size_bytes = SharedMemorySize();
      // 2.2 size 非 0 且不一致时报错退出防止影响别人已经申请好的共享内存
      if (current_size_bytes != 0 && current_size_bytes != desired_size_bytes) {
        LOG_FATAL << "Unexpected shared memory size [" << current_size_bytes << "] for desired size ["
                  << desired_size_bytes << "]";
        return false;
      }

      // 2.2 size 为 0 时分配内存
      if (current_size_bytes == 0) {
        LOG_INFO << "Resize shared memory from [" << current_size_bytes << "] to [" << desired_size_bytes << "]";
        shared_memory_->truncate(desired_size_bytes);
      }

      // 2.3 建立映射并释放文件锁
      current_size_bytes = SharedMemorySize();
      CHECK_EQ(current_size_bytes, desired_size_bytes);
      LOG_INFO << "Open shared memory [" << shared_memory_key_ << "] with size [" << current_size_bytes
               << "] successfully!";
      mapped_region_ =
          std::make_unique<boost::interprocess::mapped_region>(*shared_memory_, ::boost::interprocess::read_only);
      is_available_ = true;
      LOG_INFO << "Trying to unlock [" << file_lock_path_ << "]";
      file_lock.unlock();
      LOG_INFO << "Unlock [" << file_lock_path_ << "] successfully";
      return true;
    }
  }

  /**
   * @brief 打开并检查共享内存的 size 是否一致
   *
   * @return true
   * @return false
   */
  void OpenSharedMemory2() {
    if (is_readonly_) {
      // readonly 模式下支持延后映射共享内存空间
      try {
        shared_memory_ = std::make_unique<::boost::interprocess::shared_memory_object>(
            ::boost::interprocess::open_only, shared_memory_key_.c_str(), ::boost::interprocess::read_only);
      } catch (boost::interprocess::interprocess_exception& e) {
        LOG_WARN << "Failed to open shared memory with error: " << e.what();
        return;
      }
    } else {
      try {
        shared_memory_ = std::make_unique<::boost::interprocess::shared_memory_object>(
            ::boost::interprocess::open_or_create, shared_memory_key_.c_str(), ::boost::interprocess::read_write);
      } catch (boost::interprocess::interprocess_exception& e) {
        LOG_FATAL << e.what();
      }
    }

    // 检查共享内存大小
    CHECK_NOTNULL(shared_memory_);
    std::int64_t desired_size_bytes = shared_memory_size_mb_ << 20;  // * 1024 * 1024
    std::int64_t current_size_bytes = 0;
    shared_memory_->get_size(current_size_bytes);

    if (current_size_bytes == desired_size_bytes) {
      LOG_INFO << "Open shared memory successfully, which size is [" << current_size_bytes << "] bytes";
      if (is_readonly_) {
        mapped_region_ =
            std::make_unique<boost::interprocess::mapped_region>(*shared_memory_, ::boost::interprocess::read_only);
      } else {
        mapped_region_ =
            std::make_unique<boost::interprocess::mapped_region>(*shared_memory_, ::boost::interprocess::read_write);
      }
      is_available_ = true;
      return;
    }
    CHECK_NOTNULL(mapped_region_);

    // 共享内存大小不一致
    LOG_WARN << "Unexpected shared memory size [" << current_size_bytes << "] for desired size [" << desired_size_bytes
             << "]";
    if (is_readonly_) {
      LOG_WARN << "Waiting for subsequent re-creation of appropriately sized shared memory";
      return;
    } else {
      if (!::boost::filesystem::exists(file_lock_path_)) {
        ::boost::interprocess::ipcdetail::file_wrapper file(::boost::interprocess::open_or_create,
                                                            file_lock_path_.c_str(), ::boost::interprocess::read_write);
      }
      // 文件锁避免多进程并发创建共享内存/申请共享内存空间
      ::boost::interprocess::file_lock file_lock(file_lock_path_.c_str());
      file_lock.lock();
      LOG_INFO << "Lock [" << file_lock_path_ << "] to creating or resize shared memory";

      shared_memory_->get_size(current_size_bytes);

      if (current_size_bytes != desired_size_bytes) {
        LOG_INFO << "Resize shared memory from [" << current_size_bytes << "] to [" << desired_size_bytes << "]";
        if (current_size_bytes != 0) {
          // 通过 remove 的方式将数据清零
          boost::interprocess::shared_memory_object::remove(shared_memory_key_.c_str());
          shared_memory_ = std::make_unique<::boost::interprocess::shared_memory_object>(
              ::boost::interprocess::open_or_create, shared_memory_key_.c_str(), ::boost::interprocess::read_write);
        }
        shared_memory_->truncate(desired_size_bytes);
      }
      mapped_region_ =
          std::make_unique<boost::interprocess::mapped_region>(*shared_memory_, ::boost::interprocess::read_write);
      is_available_ = true;
      LOG_INFO << "Unlock [" << file_lock_path_ << "]";
      file_lock.unlock();
      return;
    }
  }

 private:
  int64_t SharedMemorySize() {
    int64_t shm_size = 0;
    this->shared_memory_->get_size(shm_size);
    return shm_size;
  }

 private:
  std::string shared_memory_key_;
  std::string file_lock_path_;
  uint32_t shared_memory_size_mb_ = 0;
  const bool is_readonly_;
  bool is_available_ = false;  // read_only_ 下 共享内存 shared_memory_ 是否可用, 用于延后创建和映射共享内存, 如果上次
                               // read 失败的话也置为 false 重新映射 (可能是生产者重新分配了共享内存空间)

  std::queue<Node> nodes_;
  std::unique_ptr<::boost::interprocess::shared_memory_object> shared_memory_ = nullptr;
  std::unique_ptr<::boost::interprocess::mapped_region> mapped_region_ = nullptr;

 private:
  uint64_t total_size_bytes_ = 0;  // 总的共享内存大小 (非 read_only_ 时有效)
  uint64_t free_size_bytes_ = 0;   // 剩余可用内存大小 (非 read_only_ 时有效)
  uint64_t free_offset_ = 0;       // 当前剩余内存的指针 (非 read_only_ 时有效)
};

}  // namespace lock_free
}  // namespace cpputil
