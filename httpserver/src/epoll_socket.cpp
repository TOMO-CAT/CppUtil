#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "epoll_socket.h"
#include "util/macro_util.h"
#include "logger.h"

namespace httpserver {

// 设置fd对应的文件为非阻塞
int EpollSocket::set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        flags = 0;
        perror2console("fcntl");
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// 接收一个套接字中已建立的连接
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

int EpollSocket::listen_on(int port, int backlog) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror2console("socket");
        exit(1);
    }

    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    int opt = -1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(socket_fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror2console("bind");
        exit(1);
    }

    if (listen(socket_fd, backlog) == -1) {
        perror2console("listen");
        exit(1);
    }

    log_info("start listening on port: %d", port);
    return socket_fd;
}

int EpollSocket::close_and_release(int& epoll_fd, epoll_event& event, EpollSocketWatcher& watcher) {
    if (event.data.ptr == nullptr) {
        return 0;
    }

    EpollContext* ctx = reinterpret_cast<EpollContext*>(event.data.ptr);
    watcher.OnClose(*ctx);

    int fd = ctx->fd;
    event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);

    delete ctx;
    event.data.ptr = nullptr;

    int ret = close(fd);
    log_info("close connection, fd:%d ret:%d", fd, ret);
    return ret;
}

int EpollSocket::handle_accept_event(int& epoll_fd, epoll_event& event, EpollSocketWatcher& watcher) {
    int socket_fd = event.data.fd;

    std::string client_ip;
    int conn_socket = accept_socket(socket_fd, client_ip);
    if (conn_socket == -1) {
        log_error_t(ErrorTag, "accept socket fail, client_ip:%s socket_fd:%d conn_socket:%d", client_ip.c_str(), socket_fd, conn_socket);
        return -1;
    }
    set_nonblocking(conn_socket);

    EpollContext* epoll_ctx = new EpollContext();
    epoll_ctx->fd = conn_socket;
    epoll_ctx->client_ip = client_ip;
    watcher.OnAccept(*epoll_ctx);

    struct epoll_event conn_socket_event;
    conn_socket_event.events = EPOLLIN | EPOLLET;
    conn_socket_event.data.ptr = epoll_ctx;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_socket, &conn_socket_event) == -1) {
        perror2console("epoll_ctl");
        exit(1);
    }

    return 0;
}

int EpollSocket::handle_readable_event(int& epoll_fd, epoll_event& event, EpollSocketWatcher& watcher) {
    EpollContext* epoll_ctx = reinterpret_cast<EpollContext*>(event.data.ptr);
    int fd = epoll_ctx->fd;
    char read_buffer[SS_READ_BUFFER_SIZE];
    memset(read_buffer, 0, SS_READ_BUFFER_SIZE);

    int read_size = recv(fd, read_buffer, SS_READ_BUFFER_SIZE, 0);
    int handle_ret = 0;
    if (read_size > 0) {
        log_info("read size:%d", read_size);
        handle_ret = watcher.OnReadable(*epoll_ctx, read_buffer, SS_READ_BUFFER_SIZE, read_size);
    }

    if (read_size <= 0 || handle_ret < 0) {
        close_and_release(epoll_fd, event, watcher);
        return 0;
    }

    if (handle_ret == READ_CONTINUE) {
        event.events = EPOLLIN | EPOLLET;
    } else {
        event.events = EPOLLOUT | EPOLLET;
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
    return 0;
}

int EpollSocket::handle_writeable_event(int& epoll_fd, epoll_event& event, EpollSocketWatcher& watcher) {
    EpollContext* epoll_ctx = reinterpret_cast<EpollContext*>(event.data.ptr);
    int fd = epoll_ctx->fd;

    int ret = watcher.OnWriteable(*epoll_ctx);
    if (ret == WRITE_CONN_CLOSE) {
        close_and_release(epoll_fd, event, watcher);
        return 0;
    }

    if (ret == WRITE_CONN_CONTINUE) {
        event.events = EPOLLOUT | EPOLLET;
    } else {
        event.events = EPOLLIN | EPOLLET;
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
    return 0;
}

int EpollSocket::Start(int port, EpollSocketWatcher& watcher, int backlog, int max_events) {
    int socket_fd = listen_on(port, backlog);

    int epoll_fd = epoll_create(1024);
    if (epoll_fd == -1) {
        perror2console("epoll_create");
        exit(1);
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = socket_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
        perror2console("epoll_ctl");
        exit(1);
    }

    epoll_event* events = new epoll_event[max_events];

    while (true) {
        int fd_num = epoll_wait(epoll_fd, events, max_events, -1);
        if (fd_num == -1) {
            perror2console("epoll_wait");
            exit(1);
        }

        for (int i = 0; i < fd_num; i++) {
            if (events[i].data.fd == socket_fd) {
                // accept connection
                handle_accept_event(epoll_fd, events[i], watcher);
            } else if (events[i].data.fd & EPOLLIN) {
                // readable
                handle_readable_event(epoll_fd, events[i], watcher);
            } else if (events[i].data.fd & EPOLLOUT) {
                // writeable
                handle_writeable_event(epoll_fd, events[i], watcher);
            } else {
                log_warn("unknown events: %d", events[i].events);
            }
        }
    }

    if (events != nullptr) {
        delete []events;
        events = nullptr;
    }
}

}  // namespace httpserver