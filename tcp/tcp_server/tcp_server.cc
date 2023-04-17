#include "tcp/tcp_server/tcp_server.h"

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "logger/log.h"

namespace {

constexpr uint32_t kBacklog = 10;
constexpr uint32_t kReadBufferSize = 1024;

}  // namespace

namespace tcp {

TcpServer::~TcpServer() {
  if (epoll_thread_.joinable()) {
    epoll_thread_.join();
  }
}

TcpServer::TcpServer(const std::string& delim) : delim_(delim) {
}

bool TcpServer::Start(int32_t port) {
  port_ = port;

  ListenOn();
  SetNonBlocking(listen_sockfd_);
  CreateEpoll();
  AddListenSocketToEpoll();

  epoll_thread_ = std::thread(&TcpServer::HandleEpollEvent, this);
  return true;
}

void TcpServer::Stop() {
  is_stop_ = true;
  LOG_INFO << "TcpServer is going to quit, please wait";
  epoll_thread_.join();
  LOG_INFO << "TcpServer quit successfully";
}

void TcpServer::ListenOn() {
  listen_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sockfd_ == -1) {
    LOG_ERROR << "socket() fail: " << std::strerror(errno);
    exit(EXIT_FAILURE);
  }

  int32_t opt = -1;
  setsockopt(listen_sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(listen_sockfd_, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1) {
    LOG_ERROR << "bind() fail: " << std::strerror(errno);
    exit(EXIT_FAILURE);
  }

  if (listen(listen_sockfd_, kBacklog) == -1) {
    LOG_ERROR << "listen() fail: " << std::strerror(errno);
    exit(EXIT_FAILURE);
  }

  LOG_INFO << "start to listen on port: " << port_;
}

/**
 * @brief 设置非阻塞模式
 *        IO 模型: https://www.yuque.com/tomocat/fryenb/pcis7w16rtc2yxgg
 *
 * @param fd 文件描述符
 */
void TcpServer::SetNonBlocking(int32_t fd) {
  // 获取文件状态标志:
  //     fcntl: 系统调用函数, 用于对已打开的文件描述符进行各种控制操作
  //     F_GETFL: 获取打开文件的文件描述符的 flag status flags, 它是控制文件 IO 行为的一组 bit mask
  int32_t flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    LOG_ERROR << "fcntl() fail: " << std::strerror(errno);
    exit(EXIT_FAILURE);
  }

  // 设置非阻塞模式
  //     F_SETFL: 设置文件状态标志
  flags |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) == -1) {
    LOG_ERROR << "fcntl() fail: " << std::strerror(errno);
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief 创建 epoll 实例
 *
 */
void TcpServer::CreateEpoll() {
  // size 参数用于创建 epoll 实例时告诉内核需要使用多少个文件描述符
  //  linux 内核 >2.6.8 后 size 参数就被弃用, 内核会动态地申请需要的内存
  // 但该参数必须 >0 以兼容旧版本的linux内核
  epoll_fd_ = epoll_create(1024);
  if (epoll_fd_ == -1) {
    LOG_ERROR << "epoll_create() fail: " << std::strerror(errno);
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief 添加监听套接字 listen_sockfd_ 到 epoll 实例
 *
 */
void TcpServer::AddListenSocketToEpoll() {
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = listen_sockfd_;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_sockfd_, &ev) == -1) {
    LOG_ERROR << "epoll_ctl() fail: " << std::strerror(errno);
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief 处理 Epoll 事件, 包括客户端的连接请求和发送消息的请求
 *
 */
void TcpServer::HandleEpollEvent() {
  auto accept_event_handler = [this]() {
    // accept
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(struct sockaddr_in);
    int32_t conn_fd = accept(listen_sockfd_, (struct sockaddr*)&cli_addr, &cli_addr_len);
    if (conn_fd == -1) {
      LOG_ERROR << "accept() fail: " << std::strerror(errno);
      exit(EXIT_FAILURE);
    }
    std::string cli_ip = inet_ntoa(cli_addr.sin_addr);
    std::string cli_address = cli_ip + ":" + std::to_string(ntohs(cli_addr.sin_port));
    LOG_INFO << "accept client socket from " << cli_address << ", with fd: " << conn_fd;
    sock_fd_to_client_addr_[conn_fd] = cli_address;

    // set nonblocking
    SetNonBlocking(conn_fd);

    struct epoll_event conn_ev;
    conn_ev.events = EPOLLIN | EPOLLET;
    conn_ev.data.fd = conn_fd;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, conn_fd, &conn_ev) == -1) {
      LOG_ERROR << "epoll_ctl() fail: " << std::strerror(errno);
      exit(EXIT_FAILURE);
    }
  };

  auto client_msg_handler = [this](int sock_fd, int fd_cnt) {
    static char buffer[kReadBufferSize];
    memset(buffer, '\0', sizeof(buffer));

    int nbytes = read(sock_fd, buffer, kReadBufferSize);
    if (nbytes == -1) {
      // 出错
      if (errno != EAGAIN) {
        LOG_ERROR << "read() fail: " << std::strerror(errno);
        exit(EXIT_FAILURE);
      }
    } else if (nbytes == 0) {
      // 客户端断开链接
      LOG_INFO << "client " << sock_fd_to_client_addr_[sock_fd] << " disconnected";
      sock_fd_to_client_addr_.erase(sock_fd);
      close(sock_fd);
    } else {
      // 将消息转发给其他客户端
      // 因为这里只是转发, 所以我们这里不处理粘包问题, 在客户端处根据分隔符处理
      for (auto&& iter : sock_fd_to_client_addr_) {
        int fd = iter.first;
        if (fd != sock_fd) {
          if (write(fd, buffer, nbytes) == -1) {
            LOG_ERROR << "write() fail: " << std::strerror(errno);
            exit(EXIT_FAILURE);
          }
        }
      }
    }
  };

  while (!is_stop_) {
    // 等待 Epoll 事件, -1 表示不设置超时时间, 1000 表示超时 1 秒
    int fd_cnt = epoll_wait(epoll_fd_, epoll_events_, kMaxEpollEvents, 1000);
    if (fd_cnt == -1) {
      LOG_ERROR << "epoll_wait() fail: " << std::strerror(errno);
      exit(EXIT_FAILURE);
    }

    // 处理 epoll 事件
    for (int i = 0; i < fd_cnt; ++i) {
      if (epoll_events_[i].data.fd == listen_sockfd_) {
        accept_event_handler();
      } else {
        client_msg_handler(epoll_events_[i].data.fd, fd_cnt);
      }
    }
  }
}

}  // namespace tcp
