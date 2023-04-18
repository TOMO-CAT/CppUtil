#include "http/http_server/http_response.h"

#include <string>

#include "logger/log.h"

namespace http_server {

WriteStatus HttpResponse::OnWriteable(bool is_keepalive, char* buffer, int buffer_size, int* const write_size) {
  resp_buff_.read(buffer, buffer_size);
  *write_size = resp_buff_.gcount();
  if (!resp_buff_.eof()) {
    return WriteStatus::WRITE_CONTINUE;
  }
  if (is_keepalive) {
    return WriteStatus::WRITE_ALIVE;
  }
  return WriteStatus::WRITE_OVER;
}

int HttpResponse::ExportBuffer2Response(const std::string& http_version, bool is_keepalive) {
  resp_buff_ << http_version << " " << status_line.stauts_code << " " << status_line.msg << "\r\n";
  resp_buff_ << "Server: http_server/0.1"
             << "\r\n";
  if (headers.find("Content-Type") == headers.end()) {
    resp_buff_ << "Content-Type: application/json; charset=UTF-8"
               << "\r\n";
  }
  resp_buff_ << "Content-Length: " << body.size() << "\r\n";
  if (is_keepalive) {
    resp_buff_ << "COnnection: Keep-Alive"
               << "\r\n";
  } else {
    resp_buff_ << "Connection: close"
               << "\r\n";
  }
  for (auto iter = headers.begin(); iter != headers.end(); iter++) {
    resp_buff_ << iter->first << ": " << iter->second << "\r\n";
  }
  resp_buff_ << "\r\n";
  resp_buff_ << body;
  LogInfo("export buffer to response: %s", resp_buff_.str().c_str());
  return 0;
}

int HttpResponse::Rollback(int size) {
  if (resp_buff_.eof()) {
    resp_buff_.clear();
  }
  int rollback_pos = static_cast<int>(resp_buff_.tellg()) - size;
  resp_buff_.seekg(rollback_pos);
  return resp_buff_.good() ? 0 : -1;
}

}  // namespace http_server
