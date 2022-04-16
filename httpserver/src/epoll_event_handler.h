#pragma once

#include "sys/epoll.h"
#include <string>
#include <boost/noncopyable.hpp>

namespace httpserver {

struct EpollEventContext {
    void* data_ptr;
    int fd;
    std::string client_ip;
};

class EpollEventHandler : boost::noncopyable {
 public:
    virtual int OnAccept(EpollEventContext* ctx) = 0;
    virtual ReadStatus OnReadable(EpollEventContext* ctx, char* read_buffer, int buffer_size, int read_size) = 0;
    virtual WriteStatus OnWriteable(EpollEventContext* ctx) = 0;
    virtual int OnClose(EpollEventContext* ctx) = 0;
};

}  // namespace httpserver