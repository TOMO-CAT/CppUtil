#pragma once

#include <sys/epoll.h>

#include <atomic>
#include <cstdint>
#include <cstring>
#include <string>
#include <thread>
#include <unordered_map>

#include "util/macro_util.h"

namespace tcp {

class TcpServer {
 public:
  TcpServer();
  ~TcpServer();

 public:
  bool Start(int32_t port);
  void Stop();

 private:
  void ListenOn();
  void SetNonBlocking(int32_t fd);
  void CreateEpoll();
  void AddListenSocketToEpoll();
  void HandleEpollEvent();

 private:
  static constexpr uint32_t kMaxEpollEvents = 10;

 private:
  int32_t port_ = -1;
  int32_t listen_sockfd_ = -1;
  int32_t epoll_fd_ = -1;

  std::atomic<bool> is_stop_ = false;
  std::atomic<bool> is_stop_gracefully_ = false;

  struct epoll_event epoll_events_[kMaxEpollEvents];
  std::unordered_map<int32_t, std::string> sock_fd_to_client_addr_;

  std::thread epoll_thread_;

  DISALLOW_COPY_AND_ASSIGN(TcpServer);
};

}  // namespace tcp