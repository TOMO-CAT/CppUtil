#pragma once

#include <cstdint>

namespace cpputil {
namespace lock_free {

class Message {
 public:
  virtual ~Message() = default;
};

}  // namespace lock_free
}  // namespace cpputil
