#pragma once

#include "sys/epoll.h"
#include <string>

namespace httpserver {

struct EpollContext {
    void* ptr;
    int fd;
    std::string client_ip;
};

class EpollSocketWatcher {
 public:
    virtual int OnAccept(EpollContext& ctx) = 0;
    virtual int OnReadable(EpollContext& ctx, char* read_buffer, int buffer_size, int read_size) = 0;
    virtual int OnWriteable(EpollContext& ctx) = 0;
    virtual int OnClose(EpollContext& ctx) = 0;
};

}  // namespace httpserver