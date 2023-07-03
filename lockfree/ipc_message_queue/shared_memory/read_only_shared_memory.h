#pragma once

#include <memory>
#include <string>
#include <vector>

#include "lockfree/ipc_message_queue/shared_memory/shared_memory.h"

// to move to *.cpp file
#include "logger/log.h"
#include "util/time/timestamp.h"

class ReadOnlySharedMemory final : public SharedMemory {
 public:
  explicit ReadOnlySharedMemory(const std::string& shared_memory_key) : SharedMemory(true, shared_memory_key) {
    is_available_ = OpenSharedMemory();
  }
  virtual ~ReadOnlySharedMemory();

 public:
  bool Write(uint8_t* const data, const uint32_t data_len, NodeInfo* const node_info) override {
    LOG_ERROR << "Readonly shared memory [" << shared_memory_key_ << "] can't be written";
    return false;
  }

  bool Read(const NodeInfo& node_info, std::vector<uint8_t>* const data) override {
    if (!CheckIfAvailable(node_info)) {
      return false;
    }

    CHECK_NOTNULL(data);
    data->clear();
    uint8_t* mapped_region_entry = reinterpret_cast<uint8_t*>(mapped_region_->get_address());
    const uint8_t* data_entry = mapped_region_entry + node_info.offset;
    data->assign(data_entry, data_entry + node_info.length);

    return false;
  }

  bool Read(const NodeInfo& node_info, std::string* const data) override {
    if (!CheckIfAvailable(node_info)) {
      return false;
    }

    CHECK_NOTNULL(data);
    data->clear();
    uint8_t* mapped_region_entry = reinterpret_cast<uint8_t*>(mapped_region_->get_address());
    const char* data_entry = reinterpret_cast<char*>(mapped_region_entry + node_info.offset);
    data->assign(data_entry, node_info.length);

    return true;
  }

 private:
  bool OpenSharedMemory() {
    LOG_INFO << "Trying to open shared memory with [readonly] mode";

    // 共享内存文件尚未创建, 延迟访问
    try {
      shared_memory_ = std::make_unique<::boost::interprocess::shared_memory_object>(
          ::boost::interprocess::open_only, shared_memory_key_.c_str(), ::boost::interprocess::read_only);
    } catch (boost::interprocess::interprocess_exception& e) {
      LOG_WARN << "Failed to open shared memory [" << shared_memory_key_ << "] with error: " << e.what();
      return false;
    }
    CHECK_NOTNULL(shared_memory_);

    // 虽然有共享空间但是 size 为 0
    int64_t current_size_bytes = SharedMemoryByteSize();
    if (current_size_bytes == 0) {
      LOG_WARN << "Empty shared memory [" << shared_memory_key_ << "]";
      return false;
    }

    // 成功打开共享内存, 映射进程空间
    LOG_INFO << "Open shared memory [" << shared_memory_key_ << "] with size [" << current_size_bytes
             << "] successfully!";
    mapped_region_ =
        std::make_unique<boost::interprocess::mapped_region>(*shared_memory_, ::boost::interprocess::read_only);
    return true;
  }

  bool CheckIfAvailable(const NodeInfo& node_info) {
    // 共享内存不可用时重新打开并映射一次
    if (!is_available_) {
      is_available_ = OpenSharedMemory();
      if (!is_available_) {
        LOG_WARN << "Fail to read shared memory [" << shared_memory_key_ << "]";
        return false;
      }
    }

    // 数据过期
    if (util::TimestampNanoSec() > node_info.expired_timestamp_ns) {
      LOG_WARN << "Read failed from shared memory [" << shared_memory_key_ << "] because of expired data";
      return false;
    }

    // 内存不合法, 下一次 READ 时再尝试重新映射内存
    if (node_info.offset + node_info.length > mapped_region_->get_size()) {
      LOG_WARN << "Read failed from shared memory [" << shared_memory_key_ << "] because of invalid address";
      is_available_ = false;
      return false;
    }

    return true;
  }

 public:
  bool is_available_ = false;  // 共享内存是否可用, 用于延后创建和映射共享内存
};
