#pragma once

#include <string>

#include "http/http_server/epoll_event_handler.h"

namespace http_server {

const int EPOLL_SOCKET_READ_BUFFER_SIZE = 4096;
const int EPOLL_SOCKET_WRITE_BUFFER_SIZE = 4096;

class EpollSocket {
 public:
  EpollSocket(int port, int backlog, int max_events, EpollEventHandler* handler)
      : port_(port), backlog_(backlog), max_events_(max_events), event_handler_(handler) {
  }
  int Start();

 private:
  int listen_on();
  int create_epoll();
  int add_listen_socket_to_epoll();
  int start_epoll_loop();
  int handle_accept_event();
  int handle_readable_event(epoll_event* const event);
  int handle_writeable_event(epoll_event* const event);
  int close_and_release(epoll_event* const event);
  int accept_socket(int socket_fd, std::string* const client_ip);
  static int set_nonblocking(int fd);

 private:
  int epoll_fd_;
  int listen_socket_fd_;
  int port_;
  int backlog_;
  int max_events_;
  EpollEventHandler* event_handler_;

  DISALLOW_COPY_AND_ASSIGN(EpollSocket)
};

}  // namespace http_server
