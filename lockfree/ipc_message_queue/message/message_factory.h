#pragma once

#include <memory>

#include "lockfree/ipc_message_queue/message/message.h"

namespace cpputil {
namespace lock_free {

class MessageFactory {
 public:
  static std::shared_ptr<Message> NewMessage(uint8_t* const data, const uint32_t data_size);
};

}  // namespace lock_free
}  // namespace cpputil
