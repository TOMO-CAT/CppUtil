#include <sstream>
#include <cstring>
#include "curl/curl.h"
#include "http_parser.h"
#include "logger.h"

namespace {
    const int MAX_REQ_SIZE = 10485760;
}

namespace httpserver {

std::string RequestParam::GetParam(const std::string& param) {
    auto iter = param2value.find(param);
    if (iter == param2value.end()) {
        return "";
    }
    return iter->second;
}

void RequestParam::GetParams(const std::string& param, std::vector<std::string>& values) {
    auto res = param2value.equal_range(param);
    for (auto iter = res.first; iter != res.second; ++iter) {
        values.push_back(iter->second);
    }
}

int RequestParam::ParseUrl(const std::string& url) {
    std::stringstream query_ss(url);
    log_info("parse url: %s", url.c_str());

    while (query_ss.good()) {
        std::string kv;
        std::getline(query_ss, kv, '&');
        log_info("get kv: %s", kv.c_str());

        std::stringstream kv_ss(kv);
        while (kv_ss.good()) {
            std::string key, value;
            std::getline(kv_ss, key, '=');
            std::getline(kv_ss, value, '=');
            param2value.insert({key, value});
        }
    }

    return 0;
}

std::string RequestLine::GetRequestUri() {
    std::stringstream ss(request_url);
    std::string uri;
    std::getline(ss, uri, '?');
    return uri;
}

int RequestLine::ParseRequestLine(const char* line, int size) {
    std::string line_str(line, size);
    std::stringstream ss(line_str);

    std::getline(ss, method, ' ');
    if (!ss.good()) {
        log_error("parse method error, line:%s", line_str.c_str());
        return -1;
    }
    std::getline(ss, request_url, ' ');
    if (!ss.good()) {
         log_error("parse request url fail, line:%s", line_str.c_str());
         return -1;
    }
    int ret = parse_request_url_param();
    if (ret != 0) {
        log_error("parse request url params fail, line:%s", line_str.c_str());
        return ret;
    }
    std::getline(ss, http_version, ' ');

    log_info("parse request line, method:%s url:%s verison:%s", method.c_str(), request_url.c_str(), http_version.c_str());
    return 0;
}

int RequestLine::parse_request_url_param() {
    std::stringstream ss(request_url);

    std::string uri;
    std::getline(ss, uri, '?');
    if (ss.good()) {
        std::string query_url;
        std::getline(ss, query_url, '?');
        param.ParseUrl(query_url);
    }
    return 0;
}

Request::Request() {
    parse_part = ParsePart::PARSE_REQ_LINE;
    req_buff_ = new std::stringstream();
    total_req_size_ = 0;
}

Request::~Request() {
    if (req_buff_) {
        delete req_buff_;
        req_buff_ = nullptr;
    }
}

std::string Request::get_param(std::string name) {
    if (line.method == "GET") {
        return line.GetRequestParam().GetParam(name);
    }
    if (line.method == "POST") {
        return body.GetParam(name);
    }
    return "";
}

std::string Request::get_unescape_param(std::string name) {
    std::string param = get_param(name);
    if (param.empty()) {
        return param;
    }
    char* escape_content = ::curl_unescape(param.c_str(), param.size());
    std::string unescape_param(escape_content);
    curl_free(escape_content);
    return unescape_param;
}

void Request::get_params(std::string& name, std::vector<std::string>& params) {
    if (line.method == "GET") {
        line.GetRequestParam().GetParams(name, params);
    }
    if (line.method == "POST") {
        body.GetParams(name, params);
    }
}

void Request::add_header(std::string& name, std::string& value) {
    headers_[name] = value;
}

std::string Request::get_header(const std::string& name) {
    return headers_[name];
}

std::string Request::get_request_uri() {
    return line.GetRequestUri();
}

bool Request::check_req_over() {
    const int CheckNum = 4;
    req_buff_->seekg(-CheckNum, req_buff_->end);
    char check_buff[CheckNum];
    ::bzero(check_buff, CheckNum);
    if (strncmp(check_buff, "\r\n\r\n", CheckNum) != 0) {
        log_error("read request not over!");
        return false;
    }
    req_buff_->seekg(0);
    return true;
}

int Request::parse_request(const char* read_buffer, int read_size) {
    total_req_size_ += read_size;
    if (total_req_size_ > MAX_REQ_SIZE) {
        log_error("too big request, refuse it");
        return -1;
    }
    req_buff_->write(read_buffer, read_size);

    log_info("read from client, size:%d content:%s", read_size, read_buffer);
    if (total_req_size_ < 4) {
        return 1;
    }
    bool is_over = check_req_over();
    if (!is_over) {
        return 1;  // to be continue
    }

    std::string line;
    int ret = 0;
    while (req_buff_->good()) {
        std::getline(*req_buff_, line, '\n');
        if (line == "\r") {  // the last line in head
            parse_part = PARSE_REQ_OVER;
            if (this->line.method == "POST") {  // post request need body
                parse_part = PARSE_REQ_BODY;
            }
            continue;
        }

        if (parse_part == PARSE_REQ_LINE) {  // parse request line like "GET /index.jsp HTTP/1.1"
            log_info("start parse request line, line:%s", line.c_str());
            ret = this->line.ParseRequestLine(line.c_str(), line.size() - 1);
            if (ret != 0) {
                log_error("parse request line fail");
                return -1;
            }
            parse_part = PARSE_REQ_HEAD;
            log_info("parse request line successfully, method:%s, url:%s http_version:%s",
                this->line.method.c_str(), this->line.request_url.c_str(), this->line.http_version.c_str());
            if (this->line.method != "GET" && this->line.method != "POST") {
                log_error("unsupported method, method:%s", this->line.method.c_str());
                return -1;
            }
            continue;
        }

        if (parse_part == PARSE_REQ_HEAD && !line.empty()) {  // read head
            log_info("start parse request head, line:%s", line.c_str());
            std::vector<std::string> parts;
        }
    }
}

Response::Response(CodeMsg status) {
    code_msg = status;
    is_writted = false;
}

void Response::set_head(const std::string& name, const std::string& value) {
    headers_[name] = value;
}

// void Response::set_body(Json::Value& body) {
//     Json::FastWriter writer;
//     std::string str_value = writer.write(body);
//     this->body = str_value;
// }

int Response::gen_response(std::string& http_version, bool is_keepalive) {
    log_info("start generate response, code:%d msg:%s", code_msg.status, code_msg.msg.c_str());
    res_bytes << http_version << " " << code_msg.status << " " << code_msg.msg << "\r\n";
    res_bytes << "Server: HttpServer/0.1" << "\r\n";
    if (headers_.find("Content-Type") == headers_.end()) {
        res_bytes << "Content-Type: application/json; charset=UTF-8" << "\r\n";
    }
    res_bytes << "Content-Length: " << body.size() << "\r\n";
    std::string conn_status = "Connection: close";
    if (is_keepalive) {
        conn_status = "Connection: Keep-Alive";
    }
    res_bytes << conn_status << "\r\n";
    for (auto iter = headers_.begin(); iter != headers_.end(); iter++) {
        res_bytes << iter->first << ": " << iter->second << "\r\n";
    }
    res_bytes << "\r\n";
    res_bytes << body;

    log_info("generate response content:%s", res_bytes.str().c_str());
    return 0;
}

int Response::readsome(char* buffer, int buffer_size, int& read_size) {
    res_bytes.read(buffer, buffer_size);
    read_size = res_bytes.gcount();
    if (!res_bytes.eof()) {
        return 1;
    }
    return 0;
}

int Response::rollback(int num) {
    if (res_bytes.eof()) {
        res_bytes.clear();
    }
    int rb_pos = static_cast<int>(res_bytes.tellg()) - num;
    res_bytes.seekg(rb_pos);
    return res_bytes.good() ? 0 : -1;
}


}  // namespace httpserver