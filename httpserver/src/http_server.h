#pragma once

#include <string>
#include <unordered_map>
#include "epoll_socket_watcher.h"
#include "http_parser.h"

namespace httpserver {

typedef void (*HttpHandler)(Request& request, Response& response);
// typedef void (*JsonHandler)(Request& request, Json::Value& response);

struct HttpMethod {
    int code;
    std::string name;
};

struct Resource {
    HttpMethod method;
    HttpHandler http_handler;
    // JsonHandler json_handler;
};

static const HttpMethod GET_METHOD = {1, "GET"};

class HttpEpollWatcher : public EpollSocketWatcher {
 public:
    virtual ~HttpEpollWatcher() {}

 public:
    int OnAccept(EpollContext& ctx) override;
    int OnReadable(EpollContext& ctx, char* read_buffer, int buffer_size, int read_size) override;
    int OnWriteable(EpollContext& ctx) override;
    int OnClose(EpollContext& ctx) override;

 public:
    void AddMapping(const std::string& path, HttpHandler handler, HttpMethod method = GET_METHOD);
    // void AddMapping(const std::string& path, JsonHandler handler, HttpMethod method = GET_METHOD);
    int HandleRequest(Request& request, Response& response);

 private:
    std::unordered_map<std::string, Resource> resource_map_;
};

class HttpServer {
 private:
    HttpEpollWatcher watcher_;
 public:
    void AddMapping(std::string path, HttpHandler handler, HttpMethod method = GET_METHOD);
    // void AddMapping(std::string path, JsonHandler handler, HttpMethod method = GET_METHOD);
    int Start(int port, int backlog = 10, int max_events = 1000);
};

}  // namespace httpserver