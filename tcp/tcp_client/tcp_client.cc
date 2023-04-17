#include "tcp/tcp_client/tcp_client.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <sstream>

#include "logger/log.h"

namespace tcp {

namespace {

void Split(const std::string& msg, const char delim, std::list<std::string>* const res) {
  res->clear();
  std::istringstream iss(msg);
  std::string temp;
  while (!iss.eof()) {
    std::getline(iss, temp, delim);
    if (temp != "\n" && !temp.empty()) {
      res->emplace_back(temp);
    }
  }
}

}  // namespace

TcpClient::TcpClient(const std::string& name) : name_(name) {
}

TcpClient::~TcpClient() {
  if (recv_thread_.joinable()) {
    recv_thread_.join();
  }
}

void TcpClient::Connect(const std::string& server_ip, uint32_t server_port) {
  sockfd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd_ == -1) {
    LOG_ERROR << "socket fail(): " << std::strerror(errno);
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
  server_addr.sin_port = htons(server_port);

  if (connect(sockfd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    LOG_ERROR << "connect() fail: " << std::strerror(errno);
    close(sockfd_);
    exit(EXIT_FAILURE);
  }

  LOG_INFO << "access to the [" << server_ip << ":" << server_port << "] successfully!";

  recv_thread_ = std::thread([this]() {
    while (!is_stop_) {
      // 等待数据可读
      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(sockfd_, &read_fds);

      // 设置超时时间为 1 秒
      struct timeval timeout;
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      int ret = select(sockfd_ + 1, &read_fds, nullptr, nullptr, &timeout);

      // 出错
      if (ret == -1) {
        LOG_ERROR << "[" << name_ << "]: select() fail: " << std::strerror(errno);
        exit(EXIT_FAILURE);
      }

      // 超时
      if (ret == 0) {
        LOG_WARN << "[" << name_ << "]: read timeout";
        continue;
      }

      // 数据可读
      static char buffer[kMaxBufferSize];
      memset(buffer, '\0', sizeof(buffer));
      auto nbytes = read(sockfd_, buffer, kMaxBufferSize);

      if (nbytes == -1) {
        // 出错
        LOG_ERROR << "[" << name_ << "]: read() fail: " << std::strerror(errno);
        exit(EXIT_FAILURE);
      } else if (nbytes == 0) {
        // 对端关闭连接
        LOG_INFO << "[" << name_ << "]: peer closed";
        close(sockfd_);
        exit(EXIT_FAILURE);
      } else {
        buffer[nbytes] = '\0';
        // 成功读到数据, 处理 TCP 的粘包问题(即同时收到多条消息)
        std::list<std::string> msg_list;
        Split(buffer, '\n', &msg_list);
        for (auto&& msg : msg_list) {
          LOG_INFO << "[" << name_ << "]: receive message: " << msg;
          // 将收到的消息打印到标准输出
          std::cout << msg << std::endl;
        }
      }
    }
  });
}

void TcpClient::Disconnect() {
  is_stop_ = true;
  LOG_INFO << "TcpClient is going to quit, please wait";
  recv_thread_.join();
  LOG_INFO << "TcpClient quit successfully";
}

void TcpClient::Send(const Buffer& buffer) {
  if (write(sockfd_, buffer.buf.data(), buffer.len) == -1) {
    LOG_ERROR << "write() fail: " << std::strerror(errno);
    exit(EXIT_FAILURE);
  }
}

}  // namespace tcp