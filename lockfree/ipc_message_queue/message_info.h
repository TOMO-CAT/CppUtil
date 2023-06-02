#pragma once

#include <cstdint>

namespace cpputil {
namespace lock_free {

struct MessageInfo {
  uint32_t source_process_id = 0;
};

}  // namespace lock_free
}  // namespace cpputil
