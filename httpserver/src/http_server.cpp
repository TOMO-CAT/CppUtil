#include <sys/socket.h>
#include <cstring>
#include "util/macro_util.h"
#include "http_server.h"
#include "epoll_socket.h"

namespace {
const int SS_WRITE_BUFFER_SIZE = 4096;
}

namespace httpserver {

int HttpServer::Start(int port, int backlog, int max_events) {
    EpollSocket epoll_socket;
    epoll_socket.Start(port, watcher_, backlog, max_events);
    return 0;
}

void HttpServer::AddMapping(std::string path, HttpHandler handler, HttpMethod method) {
    watcher_.AddMapping(path, handler, method);
}

// void HttpServer::AddMapping(std::string path, JsonHandler handler, HttpMethod method) {
//     watcher_.AddMapping(path, handler, method);
// }

void HttpEpollWatcher::AddMapping(const std::string& path, HttpHandler handler, HttpMethod method) {
    resource_map_[path] = {method, handler};
}

// void HttpEpollWatcher::AddMapping(const std::string& path, JsonHandler handler, HttpMethod method) {
//     resource_map_[path] = {method, nullptr, handler};
// }

int HttpEpollWatcher::HandleRequest(Request& request, Response& response) {
    std::string uri = request.get_request_uri();
    if (resource_map_.find(uri) == resource_map_.end()) {
        response.code_msg = STATUS_NOT_FOUND;
        response.body = STATUS_NOT_FOUND.msg;
        log_error("page not found, uri:%s", uri.c_str());
        return 0;
    }

    Resource src = resource_map_[request.get_request_uri()];
    HttpMethod method = src.method;
    if (method.name != request.line.method) {
        response.code_msg = STATUS_METHOD_NOT_ALLOWED;
        response.set_head("Allow", method.name);
        response.body.clear();
        log_info("not allow method, allowed:%s request method:%s", method.name.c_str(), request.line.method.c_str());
        return 0;
    }

    // if (src.json_handler != nullptr) {
    //     Json::Value root;
    //     src.json_handler(request, root);
    //     response.set_body(root);
    // } else if (src.http_handler != nullptr) {
    //     src.http_handler(request, response);
    // }
    src.http_handler(request, response);

    log_info("handle request successfully, code:%d msg:%s", response.code_msg.status, response.code_msg.msg.c_str());
    return 0;
}

int HttpEpollWatcher::OnAccept(EpollContext& ctx) {
    int conn_socket = ctx.fd;
    ctx.ptr = new HttpContext(conn_socket);
    return 0;
}

int HttpEpollWatcher::OnReadable(EpollContext& ctx, char* read_buffer, int buffer_size, int read_size) {
    HttpContext* http_ctx = reinterpret_cast<HttpContext*>(ctx.ptr);
    if (http_ctx->get_request().parse_part == PARSE_REQ_LINE) {
        http_ctx->record_start_time();
    }

    int ret = http_ctx->get_request().parse_request(read_buffer, read_size);
    if (ret != 0) {
        return ret;
    }
    HandleRequest(http_ctx->get_request(), http_ctx->get_response());
    return 0;
}

int HttpEpollWatcher::OnWriteable(EpollContext& ctx) {
    int fd = ctx.fd;
    HttpContext* http_ctx = reinterpret_cast<HttpContext*>(ctx.ptr);
    Response& resp = http_ctx->get_response();
    bool is_keepalive = ::strcasecmp(http_ctx->get_request().get_header("Connection").c_str(), "keep-alive") == 0;
    if (!resp.is_writted) {
        resp.gen_response(http_ctx->get_request().line.http_version, is_keepalive);
        resp.is_writted = true;
    }

    char buffer[SS_WRITE_BUFFER_SIZE];
    ::bzero(buffer, SS_WRITE_BUFFER_SIZE);
    int read_size = 0;

    // 1. read some response bytes
    int ret = resp.readsome(buffer, SS_WRITE_BUFFER_SIZE, read_size);
    // 2. write bytes to socket
    int n_write = send(fd, buffer, read_size, 0);
    if (n_write < 0) {
        perror2console("send");
        return WRITE_CONN_CLOSE;
    }
    // 3. when not write all buffer, we will rollback write index
    if (n_write < read_size) {
        resp.rollback(read_size - n_write);
    }
    log_info("send complete, write_num:%d read_size:%d", n_write, read_size);
    bool print_access_log = true;

    if (ret == 1) {  // not send over
        print_access_log = false;
        log_info("has huge response, we will send part first and send other part later...");
        return WRITE_CONN_CONTINUE;
    }

    if (print_access_log) {
        http_ctx->print_access_log(ctx.client_ip);
    }
    if (is_keepalive && n_write > 0) {
        http_ctx->clear();
        return WRITE_CONN_ALIVE;
    }
    return WRITE_CONN_CLOSE;
}

int HttpEpollWatcher::OnClose(EpollContext& ctx) {
    if (ctx.ptr == nullptr) {
        return 0;
    }
    HttpContext* http_ctx = reinterpret_cast<HttpContext*>(ctx.ptr);
    if (http_ctx) {
        delete http_ctx;
        http_ctx = nullptr;
    }
    return 0;
}

}  // namespace httpserver