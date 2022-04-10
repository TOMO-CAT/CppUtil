#pragma once

#include <sys/time.h>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include "logger.h"

namespace httpserver {

struct CodeMsg {
    int status;
    std::string msg;
};

static const CodeMsg STATUS_OK = {200, "OK"};
static const CodeMsg STATUS_NOT_FOUND = {404, "Not Found"};
static const CodeMsg STATUS_METHOD_NOT_ALLOWED = {405, "Method Not Allowed"};

enum ParsePart {
    PARSE_REQ_LINE = 0,
    PARSE_REQ_HEAD = 1,
    PARSE_REQ_BODY = 2,
    PARSE_REQ_OVER = 3,
};

struct RequestParam {
 public:
    std::string GetParam(const std::string& param);
    void GetParams(const std::string& param, std::vector<std::string>& values);
    int ParseUrl(const std::string& url);

 private:
    std::multimap<std::string, std::string> param2value;
};


struct RequestLine {
 public:
    std::string method;        // eg: GET/POST
    std::string request_url;   // eg: /foo?name=bar
    std::string http_version;  // eg: HTTP/1.1

 public:
    int ParseRequestLine(const char* line, int size);
    std::string GetRequestUri();
    RequestParam& GetRequestParam() { return param; }
    std::string ToString() {
        std::string res = "method:" + method;
        res += ",request_url:" + request_url;
        res += ",http_version:" + http_version;
        return res;
    }

 public:
    RequestParam param;
    int parse_request_url_param();
};

#define RequestBody RequestParam

class Request {
 public:
    Request();
    ~Request();

 public:
    int parse_part;
    RequestLine line;
    RequestBody body;

 public:
    std::string get_param(std::string name);
    std::string get_unescape_param(std::string name);
    void get_params(std::string& name, std::vector<std::string>& params);
    void add_header(std::string& name, std::string& value);
    std::string get_header(const std::string& name);
    std::string get_request_uri();
    bool check_req_over();
    int parse_request(const char* read_buffer, int read_size);

 private:
    std::map<std::string, std::string> headers_;
    std::stringstream* req_buff_;
    int total_req_size_;
};

class Response {
 private:
    std::map<std::string, std::string> headers_;
    std::stringstream res_bytes;

 public:
    bool is_writted;
    CodeMsg code_msg;
    std::string body;

 public:
    explicit Response(CodeMsg status = STATUS_OK);
    // Response(CodeMsg status, Json::Value& body);
    void set_head(const std::string& name, const std::string& value);
    // void set_body(Json::Value& body);
    int gen_response(std::string& http_version, bool is_keepalive);
    int readsome(char* buffer, int buffer_size, int& read_size);
    int rollback(int num);
};

class HttpContext {
 private:
    Response* resp;
    Request* req;

 public:
    int fd;
    timeval start;

    explicit HttpContext(int fd) : fd(fd) {
        req = new Request();
        resp = new Response();
    }

    inline void delete_req_resp() {
        if (req) {
            delete req;
            req = nullptr;
        }
        if (resp) {
            delete resp;
            resp = nullptr;
        }
    }

    ~HttpContext() {
        delete_req_resp();
    }

    void clear() {
        delete_req_resp();
        req = new Request();
        resp = new Response();
    }

    void record_start_time() {
        ::gettimeofday(&start, nullptr);
    }

    int get_cost_time() {
        struct timeval end;
        ::gettimeofday(&end, nullptr);
        return (end.tv_sec - start.tv_sec) * 10000000 + (end.tv_usec - start.tv_usec);
    }

    void print_access_log(const std::string& client_ip) {
        std::string method = req->line.method;
        std::string url = req->line.request_url;
        int cost_time = get_cost_time();
        log_info("%s %s status_code:%d cost_time:%dus body_size:%d client_ip:%s",
            method.c_str(), url.c_str(), resp->code_msg.status, cost_time, resp->body.size(), client_ip.c_str());
    }

    Response& get_response() {
        return *resp;
    }

    Request& get_request() {
        return *req;
    }
};

}  // namespace httpserver