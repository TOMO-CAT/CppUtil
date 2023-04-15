#pragma once

#include <algorithm>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "hiredis/hiredis.h"
#include "util/macro_util.h"

namespace iowrapper {

struct RedisCommand {
 public:
  RedisCommand() : reply(nullptr) {
  }
  ~RedisCommand() {
    if (reply) {
      freeReplyObject(reply);
      reply = nullptr;
    }
    for (auto buff : buffs) {
      delete[] buff;
      buff = nullptr;
    }
  }
  RedisCommand& append(const char* val) {
    std::string str_val(val);
    return append(str_val);
  }

  RedisCommand& append(const std::string& val) {
    char* ptr_char = new char[val.length() + 1];
    strncpy(ptr_char, val.c_str(), val.length());
    ptr_char[val.length()] = '\0';
    argv.push_back(ptr_char);
    buffs.push_back(ptr_char);
    return *this;
  }

  template <typename T>
  RedisCommand& append(T val) {
    std::string str_val = std::to_string(val);
    return append(str_val);
  }
  template <typename T, typename... Args>
  RedisCommand& append(T val, Args... args) {
    append(val).append(args...);
    return *this;
  }
  /**
   * 获取redis命令的字符串
   */
  std::string get_cmd_str() {
    std::string res;
    for (size_t i = 0; i < argv.size(); i++) {
      res += ("[" + std::string(argv[i]) + "]");
      if (i != argv.size() - 1) {
        res += " ";
      }
    }
    return res;
  }

 public:
  std::vector<const char*> argv;
  std::vector<char*> buffs;  // avoid memory leak
  redisReply* reply;

  DISALLOW_COPY_AND_ASSIGN(RedisCommand);
};

class RedisProxy {
 public:
  enum class ReturnCode {
    REPLY_UNKNOWN = -3,
    REPLY_ERROR = -2,
    CONNECTION_REFUSED = -1,
    REPLY_STRING = 1,
    REPLY_ARRAY = 2,
    REPLY_INTEGER = 3,
    REPLY_NIL = 4,
    REPLY_STATUS = 5,
  };

  struct Response {
    Response() = default;
    ReturnCode ret_code;
    std::string str;
    int64_t integer = 0;
    std::vector<std::string> array;

    DISALLOW_COPY_AND_ASSIGN(Response);
  };

 public:
  RedisProxy(const std::string& ip, int port, int conn_timeout_ms, int rw_timeout_ms);
  ~RedisProxy();
  /**
   * 执行单条redis命令, 支持可变参数
   *
   * example:
   *  1) ExecuteArgv("set", "key", "value");
   *  2) ExecuteArgv("lpush", "mylist", 1);
   */
  template <typename T, typename... Args>
  std::shared_ptr<Response> ExecuteArgv(T cmd, Args... args) {
    RedisCommand rc;
    rc.append(cmd, args...);
    return execute(rc);
  }
  /**
   * 执行单个完整的redis命令, 支持格式控制符
   *
   * example:
   *  1) Execute("set foo bar");
   *  2) Execute("set %s %s", foo, bar);
   */
  std::shared_ptr<Response> Execute(const char* format, ...);
  /**
   * 执行管道Redis命令, 只有返回true才能使用resps
   */
  bool PipeExecute(std::vector<RedisCommand>& cmds, std::vector<std::shared_ptr<Response>>& resps);

  // ===============================================管道redis命令的实现===============================================
  /**
   * 管道Set方法
   */
  bool PipeSet(const std::vector<std::string>& keys, const std::vector<std::string>& values,
               const std::vector<unsigned int>& expire_time_secs);
  /**
   * 管道Get方法
   */
  bool PipeGet(const std::vector<std::string>& keys, std::vector<std::pair<bool, std::string>>& values);

  // ===============================================常用redis命令的实现===============================================
  /**
   * Set方法
   */
  bool Set(const std::string& key, const std::string& value, unsigned int expire_time_sec = 0);
  /**
   * Get方法
   */
  bool Get(const std::string& key, std::string& value);

 private:
  bool connect();
  void close();
  std::string get_err_msg() const {
    return ctx_ == nullptr ? "ctx is null" : ctx_->errstr;
  }
  std::shared_ptr<Response> execute(RedisCommand& redis_cmd);
  std::shared_ptr<Response> parse_redis_reply(const redisReply* reply, const std::string& cmd);

 private:
  std::string ip_;
  int port_;
  int conn_timeout_ms_;
  int rw_timeout_ms_;
  redisContext* ctx_;

  DISALLOW_COPY_AND_ASSIGN(RedisProxy);
};

/**
 * 打印Redis返回值内容
 */
std::string PrintRedisResponse(const redisReply* reply);
/**
 * 打印RedisProxy::Response内容
 */
std::string PrintRedisResponse(const std::shared_ptr<iowrapper::RedisProxy::Response>& resp);

}  // namespace iowrapper