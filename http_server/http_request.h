#pragma once

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "http_server/epoll_socket.h"
#include "util/macro_util.h"

namespace httpserver {

struct HttpRequest {
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
  HttpRequest() : parse_part(PARSE_REQ_LINE), total_req_size_(0) {
  }
  ~HttpRequest() {
  }
  ReadStatus OnReadable(const char* read_buffer, int read_size);

 private:
  bool is_read_over();
  int parse_request_line(const std::string& request_line);
  int parse_url_params();
  int parse_body(const std::string& body);

  DISALLOW_COPY_AND_ASSIGN(HttpRequest)
};

}  // namespace httpserver