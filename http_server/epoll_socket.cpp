#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>

#include "http_server/epoll_socket.h"
#include "logger/logger.h"
#include "util/macro_util.h"

namespace httpserver {

// 设置fd对应的文件为非阻塞
int EpollSocket::set_nonblocking(int fd) {
  int flags = ::fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    flags = 0;
  }
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int EpollSocket::accept_socket(int socket_fd, std::string& client_ip) {
  struct sockaddr_in client_addr;
  socklen_t client_addr_size = sizeof(struct sockaddr_in);
  int conn_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &client_addr_size);
  if (conn_fd == -1) {
    perror2console("accept");
    return -1;
  }
  client_ip = inet_ntoa(client_addr.sin_addr);
  return conn_fd;
}

int EpollSocket::listen_on() {
  listen_socket_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  if (listen_socket_fd_ == -1) {
    log_error("socket() err:%s", strerror(errno));
    return -1;
  }
  // 一般而言端口释放2分钟后才能被复用, SO_REUSEADDR 可以让端口释放后就立刻被再次使用
  int opt = -1;
  setsockopt(listen_socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in my_addr;
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(port_);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  if (::bind(listen_socket_fd_, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1) {
    log_error("bind() err:%s", strerror(errno));
    return -1;
  }

  // 内核中维护着 未完成队列 和 已完成队列 两个队列
  // 前者指没有完成三次握手的队列, 后者指完成三次握手但是进程还未处理的队列
  // 前者大小由 /proc/sys/net/ipv4/tcp_max_syn_backlog 确定
  // 后者大小由backlog参数决定
  // 当已完成队列满了时, 如果再收到TCP第三次握手的ACK包, 那么Linux协议栈就会忽略这个包
  if (::listen(listen_socket_fd_, backlog_) == -1) {
    log_error("listen() err:%s", strerror(errno));
    return -1;
  }

  log_info("start listening on port: %d", port_);
  return 0;
}

int EpollSocket::create_epoll() {
  // size 参数用于创建epoll实例时告诉内核需要使用多少个文件描述符
  // linux 内核 >2.6.8 后 size 参数就被弃用, 内核会动态地申请需要的内存
  // 但该参数必须 >0 以兼容旧版本的linux内核
  epoll_fd_ = ::epoll_create(1024);
  if (epoll_fd_ == -1) {
    log_error("epoll_create() err:%s", strerror(errno));
    return -1;
  }
  log_info("create epoll successfully! epoll fd:%d", epoll_fd_);
  return 0;
}

int EpollSocket::add_listen_socket_to_epoll() {
  // epoll event 表示 epoll 事件, EPOLLIN表示对应的文件描述符可读
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = listen_socket_fd_;
  // epoll_ctl 用于控制某个文件描述符上的事件, EPOLL_CTL_ADD表示注册事件
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_socket_fd_, &ev) == -1) {
    log_error("epoll_ctl() err:%s", strerror(errno));
    return -1;
  }
  return 0;
}

int EpollSocket::start_epoll_loop() {
  int ret = 0;
  epoll_event* events = new epoll_event[max_events_];

  while (true) {
    // -1 表示不设置超时时间
    int fd_num = epoll_wait(epoll_fd_, events, max_events_, -1);
    if (fd_num == -1) {
      log_error("epoll_wait() err:%s", strerror(errno));
      ret = -1;
      break;
    }

    for (int i = 0; i < fd_num; i++) {
      if (events[i].data.fd == listen_socket_fd_) {
        handle_accept_event();
      } else if (events[i].events & EPOLLIN) {
        handle_readable_event(events[i]);
      } else if (events[i].events & EPOLLOUT) {
        handle_writeable_event(events[i]);
      } else {
        log_error("unknown events: %d", events[i].events);
      }
    }
  }

  if (events) {
    delete[] events;
    events = nullptr;
  }

  return ret;
}

int EpollSocket::handle_accept_event() {
  std::string client_ip;
  int conn_socket = accept_socket(listen_socket_fd_, client_ip);
  if (conn_socket == -1) {
    log_error("accept socket fail, client_ip:%s socket_fd:%d conn_socket:%d", client_ip.c_str(), listen_socket_fd_,
              conn_socket);
    return -1;
  }

  set_nonblocking(conn_socket);

  EpollEventContext* ctx = new EpollEventContext();
  ctx->fd = conn_socket;
  ctx->client_ip = client_ip;
  event_handler_->OnAccept(ctx);

  // Epoll 有两种触发模式: 水平触发(LT)和边缘触发(ET)
  // 前者只要存在着事件就会不断触发直到处理完成, 后者只触发一次相同事件
  // 多线程下可以使用 EPOLLONESHOT 避免不同的线程处理同一个 SOCKET 事件
  struct epoll_event conn_socket_event;
  conn_socket_event.events = EPOLLIN | EPOLLET;
  conn_socket_event.data.ptr = ctx;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, conn_socket, &conn_socket_event) == -1) {
    log_error("epoll_ctl() err:%s", strerror(errno));
    close_and_release(conn_socket_event);
    return -1;
  }

  return 0;
}

int EpollSocket::handle_readable_event(epoll_event& event) {
  EpollEventContext* ctx = reinterpret_cast<EpollEventContext*>(event.data.ptr);
  int fd = ctx->fd;
  char read_buffer[EPOLL_SOCKET_READ_BUFFER_SIZE];
  ::memset(read_buffer, 0, EPOLL_SOCKET_READ_BUFFER_SIZE);

  int read_size = ::recv(fd, read_buffer, EPOLL_SOCKET_READ_BUFFER_SIZE, 0);
  ReadStatus ret = ReadStatus::READ_ERROR;
  if (read_size > 0) {
    log_info("fd:%d read size:%d", fd, read_size);
    ret = event_handler_->OnReadable(ctx, read_buffer, EPOLL_SOCKET_READ_BUFFER_SIZE, read_size);
  }

  if (read_size <= 0 || ret == ReadStatus::READ_ERROR || ret == ReadStatus::READ_REACH_MAX_SIZE) {
    log_error("read error, ret:%d", static_cast<int32_t>(ret));
    close_and_release(event);
    return 0;
  }

  if (ret == ReadStatus::READ_CONTINUE) {
    event.events = EPOLLIN | EPOLLET;
  } else {
    event.events = EPOLLOUT | EPOLLET;
  }
  epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event);
  return 0;
}

int EpollSocket::handle_writeable_event(epoll_event& event) {
  EpollEventContext* ctx = reinterpret_cast<EpollEventContext*>(event.data.ptr);
  int fd = ctx->fd;

  WriteStatus ret = event_handler_->OnWriteable(ctx);
  if (ret == WriteStatus::WRITE_ERROR || ret == WriteStatus::WRITE_OVER) {
    close_and_release(event);
    return 0;
  }

  if (ret == WriteStatus::WRITE_CONTINUE) {
    event.events = EPOLLOUT | EPOLLET;
  } else {
    event.events = EPOLLIN | EPOLLET;
  }
  epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event);
  return 0;
}

int EpollSocket::close_and_release(epoll_event& event) {
  int ret = 0;

  if (event.data.ptr == nullptr) {
    return ret;
  }

  EpollEventContext* ctx = reinterpret_cast<EpollEventContext*>(event.data.ptr);
  event_handler_->OnClose(ctx);

  int fd = ctx->fd;
  event.events = EPOLLIN | EPOLLOUT | EPOLLET;
  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event);

  delete ctx;
  event.data.ptr = nullptr;

  if (fd > 0) {
    ret = close(fd);
  }
  log_info("close connection, fd:%d ret:%d", fd, ret);
  return ret;
}

int EpollSocket::Start() {
  int ret = 0;

  ret = listen_on();
  CHECK_RET(ret);

  ret = create_epoll();
  CHECK_RET(ret);

  ret = add_listen_socket_to_epoll();
  CHECK_RET(ret);

  return start_epoll_loop();
}

}  // namespace httpserver