#pragma once

#include <map>
#include <string>

#include "epoll_event_handler.h"
#include "http_request.h"
#include "http_response.h"

namespace httpserver {

typedef void (*HttpHandler)(HttpRequest& request, HttpResponse& response);

struct HttpContext {
    HttpRequest* req;
    HttpResponse* resp;
    int fd;

    explicit HttpContext(int fd);
    ~HttpContext();
    void Clear();
};

class HttpEpollEventHandler : public EpollEventHandler {
 public:
    virtual ~HttpEpollEventHandler() = default;

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

}  // namespace httpserver