#pragma once

#include <cstdint>

struct SharedMemoryNodeInfo {
  static constexpr std::size_t kMaxBufferSize = 256;

  char shared_memory_key[kMaxBufferSize] = {};
  int32_t process_id = 0;
  uint32_t channel_id = 0;
  uint64_t offset = 0;
  uint64_t length = 0;
  uint32_t check_sum = 0;
  uint64_t expired_timestamp = 0;
};
