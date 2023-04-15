#pragma once

#include <string>

#include "sys/epoll.h"
#include "util/macro_util.h"

namespace httpserver {

enum class ReadStatus {
  READ_ERROR = -1,
  READ_OVER = 0,
  READ_CONTINUE = 1,
  READ_REACH_MAX_SIZE = 2,
};

enum class WriteStatus {
  WRITE_ERROR = -1,    // write error, we will close the connection
  WRITE_OVER = 0,      // write done, we will close the connection
  WRITE_ALIVE = 1,     // keep alive, we will continue to use this connection
  WRITE_CONTINUE = 2,  // big response, we will continue to write
};

struct EpollEventContext {
  void* data_ptr;
  int fd;
  std::string client_ip;
};

class EpollEventHandler {
 public:
  EpollEventHandler() = default;
  virtual ~EpollEventHandler() = default;

 public:
  virtual int OnAccept(EpollEventContext* ctx) = 0;
  virtual ReadStatus OnReadable(EpollEventContext* ctx, char* read_buffer, int buffer_size, int read_size) = 0;
  virtual WriteStatus OnWriteable(EpollEventContext* ctx) = 0;
  virtual int OnClose(EpollEventContext* ctx) = 0;

  DISALLOW_COPY_AND_ASSIGN(EpollEventHandler)
};

}  // namespace httpserver