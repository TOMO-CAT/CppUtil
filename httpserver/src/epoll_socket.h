#pragma once

#include <string>
#include "epoll_socket_watcher.h"

namespace httpserver {

const char ErrorTag[] = "epoll socket error";
const int SS_READ_BUFFER_SIZE = 4096;

const int WRITE_CONN_ALIVE = 0;
const int WRITE_CONN_CLOSE = 1;
const int WRITE_CONN_CONTINUE = 2;

const int READ_OVER = 0;
const int READ_CONTINUE = 1;

class EpollSocket {
 public:
    static int Start(int port, EpollSocketWatcher& watcher, int backlog, int max_events);

 private:
    static int set_nonblocking(int fd);
    static int accept_socket(int socket_fd, std::string& client_ip);
    static int listen_on(int port, int backlog);
    static int close_and_release(int& epoll_fd, epoll_event& event, EpollSocketWatcher& watcher);
    static int handle_accept_event(int& epoll_fd, epoll_event& event, EpollSocketWatcher& watcher);
    static int handle_readable_event(int& epoll_fd, epoll_event& event, EpollSocketWatcher& watcher);
    static int handle_writeable_event(int& epoll_fd, epoll_event& event, EpollSocketWatcher& watcher);
};
}  // namespace httpserver
