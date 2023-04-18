#include "http/http_server/http_server.h"

#include <sys/socket.h>

#include <cstring>

#include "http/http_server/epoll_socket.h"

namespace http_server {

HttpServer::HttpServer(int port, int backlog, int max_events) {
  epoll_event_handler_ = new HttpEpollEventHandler();
  epoll_socket_ = new EpollSocket(port, backlog, max_events, epoll_event_handler_);
}

HttpServer::~HttpServer() {
  if (epoll_event_handler_) {
    delete epoll_event_handler_;
    epoll_event_handler_ = nullptr;
  }
  if (epoll_socket_) {
    delete epoll_socket_;
    epoll_socket_ = nullptr;
  }
}

int HttpServer::Start() {
  epoll_socket_->Start();
  return 0;
}

void HttpServer::RegisterHandler(std::string path, HttpHandler handler) {
  epoll_event_handler_->uri2handler[path] = handler;
}

}  // namespace http_server
