#pragma once

#include <memory>
#include <string>
#include <vector>

#include "boost/interprocess/mapped_region.hpp"
#include "boost/interprocess/shared_memory_object.hpp"

// to move in *.cpp file
#include "logger/log.h"

/**
 * @brief 共享内存抽象基类, 提供读写接口
 *
 */
class SharedMemory {
 public:
  SharedMemory(bool is_readonly, const std::string& shared_memory_key)
      : is_readonly_(is_readonly), shared_memory_key_(shared_memory_key) {
    CHECK(!shared_memory_key_.empty());
  }
  virtual ~SharedMemory();

 public:
  //   struct Node {
  //     uint64_t offset = 0;
  //     uint64_t length = 0;
  //     uint64_t expired_timestamp = 0;
  //   };

  struct NodeInfo {
    static constexpr std::size_t kMaxBufferSize = 256;

    char shared_memory_key[kMaxBufferSize] = {};
    int32_t process_id = 0;
    uint32_t channel_id = 0;
    uint64_t offset = 0;
    uint64_t length = 0;
    uint32_t check_sum = 0;
    uint64_t expired_timestamp_ns = 0;
  };

 public:
  virtual bool Write(uint8_t* const data, const uint32_t data_len, NodeInfo* const node_info) = 0;
  virtual bool Read(const NodeInfo& node_info, std::vector<uint8_t>* const data) = 0;
  virtual bool Read(const NodeInfo& node_info, std::string* const data) = 0;

 public:
  bool is_readonly() const {
    return is_readonly_;
  }

  const std::string& shared_memory_key() const {
    return shared_memory_key_;
  }

 protected:
  /**
   * @brief 获取共享内存大小 (单位 Bytes)
   *
   * @return int64_t
   */
  int64_t SharedMemoryByteSize() {
    CHECK_NOTNULL(shared_memory_);
    int64_t shm_size = 0;
    shared_memory_->get_size(shm_size);
    return shm_size;
  }

 protected:
  const bool is_readonly_;
  std::string shared_memory_key_;
  std::unique_ptr<::boost::interprocess::shared_memory_object> shared_memory_ = nullptr;
  std::unique_ptr<::boost::interprocess::mapped_region> mapped_region_ = nullptr;
};
