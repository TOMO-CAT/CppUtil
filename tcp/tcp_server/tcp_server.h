#pragma once

#include <sys/epoll.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "util/macro_util.h"

namespace tcp {

class TcpServer {
 public:
  explicit TcpServer(const std::string& delim);
  ~TcpServer();

 public:
  bool Start(int32_t port);
  void Stop();

 public:
  static constexpr uint32_t kMaxBufferSize = 1024;
  struct Buffer {
    std::array<char, kMaxBufferSize> buff;
    uint32_t len = 0;
  };

  void Send(const Buffer& buffer);
  void Receive(std::vector<std::string>* const msg_list);

 private:
  void ListenOn();
  void SetNonBlocking(int32_t fd);
  void CreateEpoll();
  void AddListenSocketToEpoll();
  void HandleEpollEvent();

 private:
  static constexpr uint32_t kMaxEpollEvents = 10;

 private:
  std::string delim_;

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