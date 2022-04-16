#pragma once

#include <string>
#include <boost/noncopyable.hpp>
#include "epoll_event_handler.h"

namespace httpserver {

const int EPOLL_SOCKET_READ_BUFFER_SIZE = 4096;
const int EPOLL_SOCKET_WRITE_BUFFER_SIZE = 4096;

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

class EpollSocket : boost::noncopyable {
 public:
    EpollSocket(int port, int backlog, int max_events, EpollEventHandler* handler) :
        port_(port), backlog_(backlog), max_events_(max_events), event_handler_(handler) {}
    int Start();

 private:
    int listen_on();
    int create_epoll();
    int add_listen_socket_to_epoll();
    int start_epoll_loop();
    int handle_accept_event();
    int handle_readable_event(epoll_event& event);
    int handle_writeable_event(epoll_event& event);
    int close_and_release(epoll_event& event);
    int accept_socket(int socket_fd, std::string& client_ip);
    static int set_nonblocking(int fd);

 private:
    int epoll_fd_;
    int listen_socket_fd_;
    int port_;
    int backlog_;
    int max_events_;
    EpollEventHandler* event_handler_;
};

}  // namespace httpserver
