#pragma once

#include <map>
#include <sstream>
#include <string>

#include "http/http_server/epoll_socket.h"

namespace httpserver {

struct StatusLine {
  int stauts_code;
  std::string msg;
};

const StatusLine STATUS_OK = {200, "OK"};
const StatusLine STATUS_NOT_FOUND = {404, "Not Found"};
const StatusLine STATUS_METHOD_NOT_ALLOWED = {405, "Method Not Allowed"};

struct HttpResponse {
 public:
  StatusLine status_line;
  std::map<std::string, std::string> headers;
  std::string body;
  bool is_writted;

 public:
  HttpResponse() : status_line(STATUS_OK), is_writted(false) {
  }
  WriteStatus OnWriteable(bool is_keepalive, char* buffer, int buffer_size, int* const write_size);
  int ExportBuffer2Response(const std::string& http_version, bool is_keepalive);
  int Rollback(int size);

 private:
  std::stringstream resp_buff_;
};

}  // namespace httpserver
