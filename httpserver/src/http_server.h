#pragma once

#include <string>
#include <unordered_map>
#include "epoll_socket.h"
#include "http_epoll_event_handler.h"

namespace httpserver {

class HttpServer {
 public:
    /**
     * @brief Construct a new Http Server object
     * 
     * @param port 
     * @param backlog 
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
     * @brief 启动Http服务
     * 
     * @return int 
     */
    int Start();

 private:
    HttpEpollEventHandler* epoll_event_handler_;
    EpollSocket* epoll_socket_;
};

}  // namespace httpserver