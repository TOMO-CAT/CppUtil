#pragma once

#include <array>
#include <atomic>
#include <string>
#include <thread>

namespace tcp {

class TcpClient final {
 public:
  TcpClient(const std::string& name);
  ~TcpClient();

 public:
  static constexpr uint32_t kMaxBufferSize = 4096;
  struct Buffer {
    std::array<char, kMaxBufferSize> buf;
    uint32_t len = 0;
  };

 public:
  void Connect(const std::string& server_ip, uint32_t server_port);
  void Disconnect();
  void Send(const Buffer& buffer);

 private:
  std::string name_;
  int32_t sockfd_ = -1;
  std::atomic<bool> is_stop_ = {false};
  std::thread recv_thread_;
};

}  // namespace tcp
