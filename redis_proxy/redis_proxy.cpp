#include "redis_proxy/redis_proxy.h"

#include <unistd.h>

#include "logger/logger.h"

namespace {
const char ERR_LOG_TAG[] = "RedisClientErr";
}

namespace iowrapper {

std::string PrintRedisResponse(const redisReply* reply) {
  if (!reply) {
    return "";
  }
  std::string array;
  for (size_t i = 0; i < reply->elements; i++) {
    array += std::string(reply->element[i]->str, reply->element[i]->len);
    if (i != reply->elements - 1) {
      array += " ";
    }
  }
  char buff[200];
  snprintf(buff, sizeof(buff), "ret_code=%d||str=%s||integer=%ld||array=%s", reply->type,
           std::string(reply->str, reply->len).c_str(), int64_t(reply->integer), array.c_str());
  return buff;
}

std::string PrintRedisResponse(const std::shared_ptr<iowrapper::RedisProxy::Response>& resp) {
  if (!resp) {
    return "";
  }
  std::string array;
  for (size_t i = 0; i < resp->array.size(); i++) {
    array += resp->array[i];
    if (i != resp->array.size() - 1) {
      array += " ";
    }
  }
  char buff[200];
  snprintf(buff, sizeof(buff), "ret_code=%d||str=%s||integer=%ld||array=%s", static_cast<int>(resp->ret_code),
           resp->str.c_str(), resp->integer, array.c_str());
  return buff;
}

RedisProxy::RedisProxy(const std::string& ip, int port, int conn_timeout_ms, int rw_timeout_ms)
    : ip_(ip), port_(port), conn_timeout_ms_(conn_timeout_ms), rw_timeout_ms_(rw_timeout_ms), ctx_(nullptr) {
}

RedisProxy::~RedisProxy() {
  close();
}

// 打开redis连接
bool RedisProxy::connect() {
  if (ctx_) {
    return true;
  }

  struct timeval tv;
  tv.tv_sec = conn_timeout_ms_ / 1000;
  tv.tv_usec = conn_timeout_ms_ % 1000 * 1000;
  ctx_ = redisConnectWithTimeout(ip_.c_str(), port_, tv);
  if (ctx_ == nullptr || ctx_->err) {
    LogErrorWithTag(ERR_LOG_TAG, "connect fail, errmsg:%s", get_err_msg().c_str());
    close();
    return false;
  }

  tv.tv_sec = rw_timeout_ms_ / 1000;
  tv.tv_usec = rw_timeout_ms_ % 1000 * 1000;
  int ret = redisSetTimeout(ctx_, tv);
  if (ret != 0) {
    LogErrorWithTag(ERR_LOG_TAG, "set timeout fail, errmsg:%s", get_err_msg().c_str());
    close();
    return false;
  }

  return true;
}

// 关闭redis连接
void RedisProxy::close() {
  if (ctx_) {
    redisFree(ctx_);
    ctx_ = nullptr;
  }
}

// 解析hiredis的返回值
std::shared_ptr<RedisProxy::Response> RedisProxy::parse_redis_reply(const redisReply* const reply,
                                                                    const std::string& cmd) {
  std::shared_ptr<Response> resp = std::make_shared<Response>();
  if (!reply) {
    resp->ret_code = ReturnCode::CONNECTION_REFUSED;
    LogErrorWithTag(ERR_LOG_TAG, "reply is null, errmsg:%s", get_err_msg().c_str());
    close();
    return resp;
  }

  LogInfo("execute redis command, cmd:%s reply:%s", cmd.c_str(), PrintRedisResponse(reply).c_str());

  switch (reply->type) {
    case REDIS_REPLY_STRING:
      resp->ret_code = ReturnCode::REPLY_STRING;
      resp->str = reply->str;
      return resp;
    case REDIS_REPLY_ARRAY:
      resp->ret_code = ReturnCode::REPLY_ARRAY;
      for (size_t i = 0; i < reply->elements; i++) {
        resp->array.emplace_back(reply->element[i]->str, reply->element[i]->len);
      }
      return resp;
    case REDIS_REPLY_INTEGER:
      resp->ret_code = ReturnCode::REPLY_INTEGER;
      resp->integer = reply->integer;
      return resp;
    case REDIS_REPLY_NIL:
      resp->ret_code = ReturnCode::REPLY_NIL;
      return resp;
    case REDIS_REPLY_STATUS:
      resp->ret_code = ReturnCode::REPLY_STATUS;
      resp->str = reply->str;
      return resp;
    case REDIS_REPLY_ERROR:
      LogErrorWithTag(ERR_LOG_TAG, "execute fail, errmsg:%s cmd:%s", get_err_msg().c_str(), cmd.c_str());
      resp->ret_code = ReturnCode::REPLY_ERROR;
      resp->str = reply->str;
      return resp;
    default:
      LogErrorWithTag(ERR_LOG_TAG, "uncatch redis reply type, type:%d cmd:%s", reply->type, cmd.c_str());
      resp->ret_code = ReturnCode::REPLY_UNKNOWN;
      resp->str = reply->str == nullptr ? "nullptr" : reply->str;
      return resp;
  }
}

std::shared_ptr<RedisProxy::Response> RedisProxy::execute(RedisCommand* const redis_cmd) {
  std::shared_ptr<Response> resp = std::make_shared<Response>();
  if (!connect()) {
    resp->ret_code = ReturnCode::CONNECTION_REFUSED;
    return resp;
  }

  int argc = redis_cmd->argv.size();
  const char** argv = redis_cmd->argv.data();
  redis_cmd->reply = reinterpret_cast<redisReply*>(redisCommandArgv(ctx_, argc, argv, NULL));
  return parse_redis_reply(redis_cmd->reply, redis_cmd->get_cmd_str());
}

std::shared_ptr<RedisProxy::Response> RedisProxy::Execute(const char* format, ...) {
  std::shared_ptr<Response> resp = std::make_shared<Response>();

  if (!connect()) {
    resp->ret_code = ReturnCode::CONNECTION_REFUSED;
    return resp;
  }

  // you can't use same va_list twice, or it may cause crash
  va_list args, args_copy;
  va_start(args, format);
  va_copy(args_copy, args);

  redisReply* reply = reinterpret_cast<redisReply*>(redisvCommand(ctx_, format, args));
  va_end(args);

  char buff[100];
// https://stackoverflow.com/questions/36120717/correcting-format-string-is-not-a-string-literal-warning
#if defined(__has_warning)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
  vsnprintf(buff, sizeof(buff), format, args_copy);
#if defined(__has_warning)
#pragma clang diagnostic pop
#endif
  va_end(args_copy);

  resp = parse_redis_reply(reply, buff);
  // because we don't use RAII class RedisCommand, so here may cause memory leak.
  // we must release reply by ourselves
  if (reply) {
    freeReplyObject(reply);
    reply = nullptr;
  }

  return resp;
}

bool RedisProxy::PipeExecute(std::vector<RedisCommand>* const cmds,
                             std::vector<std::shared_ptr<RedisProxy::Response>>* const resps) {
  if (!connect()) {
    return false;
  }

  int ret = 0;
  for (RedisCommand& cmd : *cmds) {
    ret = redisAppendCommandArgv(ctx_, cmd.argv.size(), cmd.argv.data(), nullptr);
    if (ret != 0) {
      LogErrorWithTag(ERR_LOG_TAG, "redisAppendCommandArgv fail, ret:%d cmd:%s", ret, cmd.get_cmd_str().c_str());
      close();
      return false;
    }
  }

  redisReply* redis_reply = nullptr;
  for (size_t i = 0; i < cmds->size(); i++) {
    ret = redisGetReply(ctx_, reinterpret_cast<void**>(&redis_reply));
    if (redis_reply == nullptr || ret != 0) {
      LogErrorWithTag(ERR_LOG_TAG, "redisGetReply fail, ret:%d is_reply_null:%d", ret, redis_reply == nullptr);
      close();
      return false;
    }
    resps->push_back(parse_redis_reply(redis_reply, (*cmds)[i].get_cmd_str()));
    freeReplyObject(redis_reply);
  }

  return true;
}

bool RedisProxy::Set(const std::string& key, const std::string& value, unsigned int expire_time_sec) {
  std::shared_ptr<Response> resp;
  if (expire_time_sec > 0) {
    resp = ExecuteArgv("set", key, value, "ex", expire_time_sec);
  } else {
    resp = ExecuteArgv("set", key, value);
  }
  if (resp->ret_code == ReturnCode::REPLY_STATUS && resp->str == "OK") {
    return true;
  }
  return false;
}

bool RedisProxy::Get(const std::string& key, std::string* const value) {
  auto resp = ExecuteArgv("get", key);
  if (resp->ret_code == ReturnCode::REPLY_STRING) {
    *value = resp->str;
    return true;
  }
  return false;
}

bool RedisProxy::PipeSet(const std::vector<std::string>& keys, const std::vector<std::string>& values,
                         const std::vector<unsigned int>& expire_time_secs) {
  if (keys.size() != values.size() || values.size() != expire_time_secs.size()) {
    LogErrorWithTag(ERR_LOG_TAG, "invalid param for PipeSet, keys.size:%d values.size:%d expires.size:%d", keys.size(),
                    values.size(), expire_time_secs.size());
    return false;
  }

  size_t cmd_cnt = keys.size();
  std::vector<RedisCommand> cmds(cmd_cnt);
  for (size_t i = 0; i < keys.size(); i++) {
    RedisCommand& cmd = cmds[i];
    cmd.append("set").append(keys[i]).append(values[i]);
    if (expire_time_secs[i] > 0) {
      cmd.append("ex");
      cmd.append(expire_time_secs[i]);
    }
  }

  std::vector<std::shared_ptr<Response>> resps;
  if (!PipeExecute(&cmds, &resps)) {
    return false;
  }

  for (const auto& resp : resps) {
    if (resp->ret_code != ReturnCode::REPLY_STATUS || resp->str != "OK") {
      LogErrorWithTag(ERR_LOG_TAG, "PipeSet fail, ret_code:%d str:%s", static_cast<int>(resp->ret_code),
                      resp->str.c_str());
      return false;
    }
  }

  return true;
}

bool RedisProxy::PipeGet(const std::vector<std::string>& keys,
                         std::vector<std::pair<bool, std::string>>* const values) {
  size_t cmd_cnt = keys.size();
  std::vector<RedisCommand> cmds(cmd_cnt);
  for (size_t i = 0; i < keys.size(); i++) {
    cmds[i].append("get").append(keys[i]);
  }
  std::vector<std::shared_ptr<Response>> resps;
  if (!PipeExecute(&cmds, &resps)) {
    return false;
  }

  for (const auto& resp : resps) {
    if (resp->ret_code == ReturnCode::REPLY_STRING) {
      values->push_back({true, resp->str});
    } else {
      values->push_back({false, ""});
    }
  }

  return true;
}

}  // namespace iowrapper