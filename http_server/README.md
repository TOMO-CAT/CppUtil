# 基于Epoll的简易Http服务器

## 简介

基于epoll的C++简易Http服务器。

## 特性

* 支持Post和Get请求
* 基于Epoll的IO复用
* 完善的日志输出
* 接口简单

## 使用方法

代码：

> <https://github.com/TOMO-CAT/CppUtil/tree/main/httpserver>

编译后生成头文件和动态库：

```bash
$make
```

简单使用：

```c++
#include "http_server.h"
#include "logger.h"
#include "jsoncpp/json.h"

void echo(httpserver::HttpRequest& req, httpserver::HttpResponse& resp) {
    std::string name = req.url_params["name"];
    Json::Value root;
    root["name"] = name;
    resp.body = root.toStyledString();
}

/**
$curl "127.0.0.1:8888/echo?name=tomocat"
{
   "name" : "tomocat"
}
 */
int main() {
    httpserver::HttpServer http_server(8888);
    http_server.RegisterHandler("/echo", echo);

    http_server.Start();
    return 0;
}
```

## 设计方案

### 1. EpollSocket: 基于Epoll的服务端Socket

相关细节：

* TCP服务端往往需要绑定一个Socket作为服务端监听端口，选择性能较佳的Epoll实现。
* 设置Socket为`SO_REUSEADDR`，让端口释放后可以立刻被再次使用。
* 内核维护着“未完成队列”和“已完成队列”两个队列，前者指未完成三次握手的队列（由`/proc/sys/net/ipv4/tcp_max_syn_backlog`确定），后者指完成三次握手但是进程还未处理的队列（由backlog参数确定）。当已完成队列满了时, 如果再收到TCP第三次握手的ACK包, 那么Linux协议栈就会忽略这个包。
* 使用单线程Reactor的模式处理EpollEvent，包括accept、readable和writeable三类事件

```c++
class EpollSocket : boost::noncopyable {
 public:
    EpollSocket(int port, int backlog, int max_events, EpollEventHandler* handler) :
        port_(port), backlog_(backlog), max_events_(max_events), event_handler_(handler) {}
    int Start();

 private:
    int listen_on();
    int create_epoll();
    int add_listen_socket_to_epoll();
    int start_epoll_loop();
    int handle_accept_event();
    int handle_readable_event(epoll_event& event);
    int handle_writeable_event(epoll_event& event);
    int close_and_release(epoll_event& event);
    int accept_socket(int socket_fd, std::string& client_ip);
    static int set_nonblocking(int fd);

 private:
    int epoll_fd_;
    int listen_socket_fd_;
    int port_;
    int backlog_;
    int max_events_;
    EpollEventHandler* event_handler_;
};
```

### 2. EpollEvent处理接口：EpollEventHandler

Epoll需要处理三类事件，为了实现各式各样的网络服务端，这里将其实现成接口：

```c++
class EpollEventHandler : boost::noncopyable {
 public:
    virtual int OnAccept(EpollEventContext* ctx) = 0;
    virtual ReadStatus OnReadable(EpollEventContext* ctx, char* read_buffer, int buffer_size, int read_size) = 0;
    virtual WriteStatus OnWriteable(EpollEventContext* ctx) = 0;
    virtual int OnClose(EpollEventContext* ctx) = 0;
};
```

在我们的Http服务端场景下，需要继承该类实现Http的EpollEventHandler：

```c++
class HttpEpollEventHandler : public EpollEventHandler {
 public:
    int OnAccept(EpollEventContext* ctx) override;
    ReadStatus OnReadable(EpollEventContext* ctx, char* read_buffer, int buffer_size, int read_size) override;
    WriteStatus OnWriteable(EpollEventContext* ctx) override;
    int OnClose(EpollEventContext* ctx) override;

 public:
    std::map<std::string, HttpHandler> uri2handler;

 private:
    int handle_http_request(HttpRequest* req, HttpResponse* resp);
};
```

### 3. Http请求与返回值

简单的Http请求包括四个部分：

* request line
* headers
* `\r\n`
* body

```c++
struct HttpRequest : boost::noncopyable {
 public:
    enum ParsePhase {
        PARSE_REQ_LINE,
        PARSE_REQ_HEAD,
        PARSE_REQ_BODY,
        PARSE_REQ_OVER,
    };
    static const int MAX_REQUEST_SIZE = 1024 * 1024 * 10;

 public:
    std::string method;        // eg: GET or POST
    std::string url;           // eg: /foo?name=bar
    std::string uri;           // eg: /foo
    std::string http_version;  // eg: HTTP/1.1
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> url_params;
    std::map<std::string, std::string> body_params;
    int parse_part;

 private:
    std::stringstream req_buff_;
    int total_req_size_;

 public:
    HttpRequest() : parse_part(PARSE_REQ_LINE), total_req_size_(0) {}
    ~HttpRequest() {}
    ReadStatus OnReadable(const char* read_buffer, int read_size);

 private:
    bool is_read_over();
    int parse_request_line(const std::string& request_line);
    int parse_url_params();
    int parse_body(const std::string& body);
};
```

Http返回值也包含四个部分：

* status line
* headers
* `\r\n`
* body

```c++
struct HttpResponse {
 public:
    StatusLine status_line;
    std::map<std::string, std::string> headers;
    std::string body;
    bool is_writted;

 public:
    HttpResponse() : status_line(STATUS_OK), is_writted(false) {}
    WriteStatus OnWriteable(bool is_keepalive, char* buffer, int buffer_size, int& write_size);
    int ExportBuffer2Response(const std::string& http_version, bool is_keepalive);
    int Rollback(int size);

 private:
    std::stringstream resp_buff_;
};
```

### 4. 最终的Http服务器

提供注册handler的函数和一个阻塞式的Start函数：

```c++
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
```

## Reference

[1] <https://github.com/hongliuliao/ehttp>
