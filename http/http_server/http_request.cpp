#include "http/http_server/http_request.h"

#include <cstring>
#include <utility>

#include "logger/log.h"
#include "util/string/string_util.h"

namespace http_server {

ReadStatus HttpRequest::OnReadable(const char* read_buffer, int read_size) {
  // Http请求报文包含如下几个部分
  // * request line
  // * header
  // * 空行 \r\n
  // * 请求数据
  total_req_size_ += read_size;
  if (total_req_size_ > MAX_REQUEST_SIZE) {
    LogError("reach max request size %d bytes, we will refuse it!", MAX_REQUEST_SIZE);
    return ReadStatus::READ_REACH_MAX_SIZE;
  }
  req_buff_.write(read_buffer, read_size);
  LogInfo("read from client, size:%d content:%s", read_size, read_buffer);

  // 保证每次都读取到一个完整的部分(http request中以 \r\n 区分)后在开始解析
  if (total_req_size_ < 4) {
    return ReadStatus::READ_CONTINUE;
  }
  if (!is_read_over()) {
    return ReadStatus::READ_CONTINUE;
  }

  std::string line;
  while (req_buff_.good()) {
    std::getline(req_buff_, line, '\n');
    LogInfo("read http request: %s", line.c_str());

    // 请求头后的空行, 意味着 header 读取结束
    if (line == "\r") {
      if (method == "POST") {
        parse_part = PARSE_REQ_OVER;
      } else {
        parse_part = PARSE_REQ_OVER;
      }
      continue;
    }

    if (parse_part == PARSE_REQ_LINE) {
      LogInfo("parse http request line:%s", line.c_str());
      if (parse_request_line(line)) {
        LogError("parse request line fail, line:%s", line.c_str());
        return ReadStatus::READ_ERROR;
      }
      LogInfo("parse http request line successfully! method:%s url:%s http_version:%s params_cnt:%d", method.c_str(),
              url.c_str(), http_version.c_str(), url_params.size());
      if (method != "POST" && method != "GET") {
        LogError("unsupported method %s", method.c_str());
        return ReadStatus::READ_ERROR;
      }
      parse_part = PARSE_REQ_HEAD;
      continue;
    }

    if (parse_part == PARSE_REQ_HEAD && !line.empty()) {
      LogInfo("parse http request header: %s", line.c_str());
      std::vector<std::string> parts;
      util::string_split(line, ':', &parts);
      if (parts.size() < 2) {
        LogError("invalid http request headers: %s", line.c_str());
        continue;
      }
      headers[parts[0]] = parts[1];
      continue;
    }

    if (parse_part == PARSE_REQ_BODY && !line.empty()) {
      LogInfo("parse http body: %s", line.c_str());
      parse_body(line);
      parse_part = PARSE_REQ_OVER;
      break;
    }
  }

  if (parse_part != PARSE_REQ_OVER) {
    LogError("parse http request incompletely, url:%s method:%s version:%s", url.c_str(), method.c_str(),
             http_version.c_str());
    return ReadStatus::READ_CONTINUE;
  }

  return ReadStatus::READ_OVER;
}

// 检查最后四个字符是否是终止符 \r\n
bool HttpRequest::is_read_over() {
  const int CHECK_SIZE = 4;

  req_buff_.seekg(-CHECK_SIZE, req_buff_.end);
  char buff[CHECK_SIZE];
  ::bzero(buff, CHECK_SIZE);
  req_buff_.readsome(buff, CHECK_SIZE);
  if (strncmp(buff, "\r\n\r\n", CHECK_SIZE)) {
    return false;
  }
  req_buff_.seekg(0);
  return true;
}

int HttpRequest::parse_request_line(const std::string& line) {
  std::stringstream ss(line);
  std::getline(ss, method, ' ');
  if (!ss.good()) {
    LogError("parse method fail, line:%s", line.c_str());
    return -1;
  }
  std::getline(ss, url, ' ');
  if (!ss.good()) {
    LogError("parse url fail, line:%s", line.c_str());
    return -1;
  }
  if (parse_url_params()) {
    LogError("parse url params fail, line:%s", line.c_str());
    return -1;
  }
  std::getline(ss, http_version, ' ');
  return 0;
}

int HttpRequest::parse_url_params() {
  std::stringstream ss(url);
  std::getline(ss, uri, '?');
  if (ss.good()) {
    std::string query_url;
    std::getline(ss, query_url, '?');

    std::stringstream query_url_ss(query_url);
    while (query_url_ss.good()) {
      std::string kv;
      std::getline(query_url_ss, kv, '&');
      LogInfo("parse url params, kv:%s", kv.c_str());

      std::stringstream kv_ss(kv);
      while (kv_ss.good()) {
        std::string key, value;
        std::getline(kv_ss, key, '=');
        std::getline(kv_ss, value, '=');
        url_params[key] = value;
      }
    }
  }
  return 0;
}

int HttpRequest::parse_body(const std::string& body) {
  // 目前支持的POST方法body格式比较简单, 后续再考虑扩展
  std::stringstream ss(body);
  while (ss.good()) {
    std::string kv;
    std::getline(ss, kv, '&');
    LogInfo("parse body params, kv:%s", kv.c_str());

    std::stringstream kv_ss(kv);
    while (kv_ss.good()) {
      std::string key, value;
      std::getline(kv_ss, key, '=');
      std::getline(kv_ss, value, '=');
      body_params.insert({key, value});
    }
  }
  return 0;
}

}  // namespace http_server
