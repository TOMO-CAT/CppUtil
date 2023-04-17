#include "http/http_server/http_epoll_event_handler.h"

#include <sys/socket.h>

#include <cstring>

#include "logger/log.h"

namespace httpserver {

HttpContext::~HttpContext() {
  if (req) {
    delete req;
    req = nullptr;
  }
  if (resp) {
    delete resp;
    resp = nullptr;
  }
}

HttpContext::HttpContext(int fd) : fd(fd) {
  req = new HttpRequest();
  resp = new HttpResponse();
}

void HttpContext::Clear() {
  if (req) {
    delete req;
  }
  if (resp) {
    delete resp;
  }
  req = new HttpRequest();
  resp = new HttpResponse();
}

int HttpEpollEventHandler::OnAccept(EpollEventContext* ctx) {
  int conn_socket = ctx->fd;
  ctx->data_ptr = new HttpContext(conn_socket);
  return 0;
}

ReadStatus HttpEpollEventHandler::OnReadable(EpollEventContext* ctx, char* read_buffer, int buffer_size,
                                             int read_size) {
  HttpContext* http_ctx = reinterpret_cast<HttpContext*>(ctx->data_ptr);
  ReadStatus read_ret = http_ctx->req->OnReadable(read_buffer, read_size);
  if (read_ret != ReadStatus::READ_OVER) {
    return read_ret;
  }
  handle_http_request(http_ctx->req, http_ctx->resp);
  return ReadStatus::READ_OVER;
}

WriteStatus HttpEpollEventHandler::OnWriteable(EpollEventContext* ctx) {
  HttpContext* http_ctx = reinterpret_cast<HttpContext*>(ctx->data_ptr);
  int socket_fd = http_ctx->fd;
  HttpRequest* req = http_ctx->req;
  HttpResponse* resp = http_ctx->resp;

  bool is_keepalive = req->headers["Connection"] == std::string("keep-alive");

  if (!resp->is_writted) {
    resp->ExportBuffer2Response(req->http_version, is_keepalive);
    resp->is_writted = true;
  }

  char buffer[EPOLL_SOCKET_WRITE_BUFFER_SIZE];
  ::bzero(buffer, EPOLL_SOCKET_WRITE_BUFFER_SIZE);
  int write_size = 0;

  WriteStatus write_ret = resp->OnWriteable(is_keepalive, buffer, EPOLL_SOCKET_WRITE_BUFFER_SIZE, &write_size);
  int n_write = ::send(socket_fd, buffer, write_size, 0);
  if (n_write < 0) {
    LogError("send fail, content:%s", buffer);
    return WriteStatus::WRITE_ERROR;
  }
  // if we don't write all buffer successfully, we will rollback the write buffer index
  if (n_write < write_size) {
    resp->Rollback(write_size - n_write);
    LogWarn("partial sending failed, n_write:%d write_size:%d content:%s", n_write, write_size, buffer);
  }
  LogInfo("send successfully! content: %s", buffer);

  if (write_ret == WriteStatus::WRITE_ALIVE && n_write > 0) {
    http_ctx->Clear();
  }
  return write_ret;
}

int HttpEpollEventHandler::OnClose(EpollEventContext* ctx) {
  if (ctx->data_ptr == nullptr) {
    return 0;
  }

  HttpContext* http_ctx = reinterpret_cast<HttpContext*>(ctx->data_ptr);
  delete http_ctx;
  http_ctx = nullptr;
  return 0;
}

int HttpEpollEventHandler::handle_http_request(HttpRequest* req, HttpResponse* resp) {
  std::string uri = req->uri;
  if (uri2handler.find(uri) == uri2handler.end()) {
    resp->status_line = STATUS_NOT_FOUND;
    resp->body = STATUS_NOT_FOUND.msg;
    LogWarn("page not found, uri:%s", uri.c_str());
    return 0;
  }

  if (req->method != "POST" && req->method != "GET") {
    resp->status_line = STATUS_METHOD_NOT_ALLOWED;
    resp->body = STATUS_METHOD_NOT_ALLOWED.msg;
    LogWarn("not allowed method, method:%s", req->method.c_str());
  }

  uri2handler[uri](req, resp);
  LogInfo("handler http request successfully! code:%d msg:%s", resp->status_line.stauts_code,
          resp->status_line.msg.c_str());
  return 0;
}

}  // namespace httpserver