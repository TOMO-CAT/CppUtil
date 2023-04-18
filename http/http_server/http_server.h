#pragma once

#include <string>
#include <unordered_map>

#include "http/http_server/epoll_socket.h"
#include "http/http_server/http_epoll_event_handler.h"

namespace http_server {

class HttpServer {
 public:
  /**
   * @brief Construct a new Http Server object
   *
   * @param port 端口
   * @param backlog TCK已完成队列的最大值
   * @param max_events
   */
  explicit HttpServer(int port, int backlog = 10, int max_events = 1000);
  ~HttpServer();

 public:
  /**
   * @brief
   *
   * @param path
   * @param handler
   */
  void RegisterHandler(std::string path, HttpHandler handler);
  /**
   * @brief 阻塞式启动Http服务
   *
   * @return int
   */
  int Start();

 private:
  HttpEpollEventHandler* epoll_event_handler_;
  EpollSocket* epoll_socket_;
};

}  // namespace http_server
