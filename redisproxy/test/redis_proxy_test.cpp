#include <iostream>
#include "redis_proxy.h"
#include "gtest/gtest.h"

/*
 * 注意: 需要在本地先启动一个redis server才可正常测试
 */
const char REDIS_IP[] = "127.0.0.1";
const int PORT = 6379;
const int CONNECTION_TIMEOUT_MS = 100;
const int RW_TIMEOUT_MS = 200;

using iowrapper::PrintRedisResponse;
using iowrapper::RedisProxy;

// =========================================ExecuteArgv=========================================
// ./bin/match.test --gtest_filter=TestCase.TestExecuteArgvString
TEST(TestCase, TestExecuteArgvString) {
    std::shared_ptr<RedisProxy::Response> resp;
    RedisProxy rp(REDIS_IP, PORT, CONNECTION_TIMEOUT_MS, RW_TIMEOUT_MS);

    resp = rp.ExecuteArgv("SET", "foo", 10);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STATUS, resp->ret_code) << iowrapper::PrintRedisResponse(resp);
    EXPECT_EQ("OK", resp->str) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("GET", "foo");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STRING, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ("10", resp->str) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("GETSET", "foo", 100);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STRING, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ("10", resp->str) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("INCR", "foo");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(101, resp->integer) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("INCRBY", "foo", 98);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(199, resp->integer) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("INCRBYFLOAT", "foo", 0.01);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STRING, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ("199.01", resp->str) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("DEL", "foo");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(1, resp->integer) << PrintRedisResponse(resp);
}

// ./bin/match.test --gtest_filter=TestCase.TestExecuteArgvHash
TEST(TestCase, TestExecuteArgvHash) {
    std::shared_ptr<RedisProxy::Response> resp;
    RedisProxy rp(REDIS_IP, PORT, CONNECTION_TIMEOUT_MS, RW_TIMEOUT_MS);
    const std::string hash_key = "my_hash";

    resp = rp.ExecuteArgv("HMSET", hash_key, "key1", "value1", "key2", "value2", "key3", "value3");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STATUS, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ("OK", resp->str) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("HEXISTS", hash_key, "key1");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(1, resp->integer) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("HSETNX", hash_key, "key1", "should be set fail");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(0, resp->integer) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("DEL", hash_key);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(1, resp->integer) << PrintRedisResponse(resp);
}

// ./bin/match.test --gtest_filter=TestCase.TestExecuteArgvList
TEST(TestCase, TestExecuteArgvList) {
    std::shared_ptr<RedisProxy::Response> resp;
    RedisProxy rp(REDIS_IP, PORT, CONNECTION_TIMEOUT_MS, RW_TIMEOUT_MS);
    const std::string key = "my_list";

    resp = rp.ExecuteArgv("LPUSH", key, "tomo");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(1, resp->integer) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("LPUSH", key, "cat");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(2, resp->integer) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("LRANGE", key, 0, 10);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_ARRAY, resp->ret_code) << PrintRedisResponse(resp);
    ASSERT_EQ(2, resp->array.size()) << PrintRedisResponse(resp);
    EXPECT_EQ("cat", resp->array[0]) << PrintRedisResponse(resp);
    EXPECT_EQ("tomo", resp->array[1]) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("DEL", key);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(1, resp->integer) << PrintRedisResponse(resp);
}

// ./bin/match.test --gtest_filter=TestCase.TestExecuteArgvSet
TEST(TestCase, TestExecuteArgvSet) {
    std::shared_ptr<RedisProxy::Response> resp;
    RedisProxy rp(REDIS_IP, PORT, CONNECTION_TIMEOUT_MS, RW_TIMEOUT_MS);
    const std::string key = "my_set";

    resp = rp.ExecuteArgv("SADD", key, "tomo", "cat", "tomocat");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(3, resp->integer) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("SADD", key, "cat");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(0, resp->integer) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("SCARD", key);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(3, resp->integer) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("DEL", key);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(1, resp->integer) << PrintRedisResponse(resp);
}

// ./bin/match.test --gtest_filter=TestCase.TestExecuteArgvSortedSet
TEST(TestCase, TestExecuteArgvSortedSet) {
    std::shared_ptr<RedisProxy::Response> resp;
    RedisProxy rp(REDIS_IP, PORT, CONNECTION_TIMEOUT_MS, RW_TIMEOUT_MS);
    const std::string key = "my_sorted_set";

    // 添加元素
    resp = rp.ExecuteArgv("ZADD", key, 100, "tomocat", 50, "tiger", 10.5, "mouse", 0, "bug");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(4, resp->integer) << PrintRedisResponse(resp);

    // 计算在指定区间内的成员数
    resp = rp.ExecuteArgv("ZCOUNT", key, 0, 10.5);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(2, resp->integer) << PrintRedisResponse(resp);

    // 更新成员分数
    resp = rp.ExecuteArgv("ZADD", key, 1000, "tiger");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(0, resp->integer) << PrintRedisResponse(resp);

    // 查看成员的分数
    resp = rp.ExecuteArgv("ZSCORE", key, "tiger");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STRING, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ("1000", resp->str) << PrintRedisResponse(resp);

    // 删除key
    resp = rp.ExecuteArgv("DEL", key);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(1, resp->integer) << PrintRedisResponse(resp);
}

// ./bin/match.test --gtest_filter=TestCase.TestExecuteArgvSpace
TEST(TestCase, TestExecuteArgvSpace) {
    std::shared_ptr<RedisProxy::Response> resp;
    RedisProxy rp(REDIS_IP, PORT, CONNECTION_TIMEOUT_MS, RW_TIMEOUT_MS);

    // ======value with space======
    const std::string key = "TestExecuteArgvSpace";

    resp = rp.ExecuteArgv("SET", key, "value with space");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STATUS, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ("OK", resp->str) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("GET", key);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STRING, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ("value with space", resp->str) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("DEL", key);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(1, resp->integer) << PrintRedisResponse(resp);

    // ========key with space========
    const std::string space_key = "key with space";

    resp = rp.ExecuteArgv("SET", space_key, "value");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STATUS, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ("OK", resp->str) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("GET", space_key);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STRING, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ("value", resp->str) << PrintRedisResponse(resp);

    resp = rp.ExecuteArgv("DEL", space_key);
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(1, resp->integer) << PrintRedisResponse(resp);
}

// =========================================Execute=========================================
// ./bin/match.test --gtest_filter=TestCase.TestExecute
TEST(TestCase, TestExecute) {
    std::shared_ptr<RedisProxy::Response> resp;
    RedisProxy rp(REDIS_IP, PORT, CONNECTION_TIMEOUT_MS, RW_TIMEOUT_MS);

    // 格式化字符串 + 空格value + 空格key
    resp = rp.Execute("set %s %s", "key with space", "value with space");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STATUS, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ("OK", resp->str) << PrintRedisResponse(resp);

    // 删除key
    resp = rp.Execute("del %s", "key with space");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_INTEGER, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ(1, resp->integer) << PrintRedisResponse(resp);

    // 单行字符串命令
    resp = rp.Execute("set foo bar ex 1");
    ASSERT_TRUE(resp != nullptr);
    EXPECT_EQ(RedisProxy::ReturnCode::REPLY_STATUS, resp->ret_code) << PrintRedisResponse(resp);
    EXPECT_EQ("OK", resp->str) << PrintRedisResponse(resp);
}

// =========================================Set Get=========================================
// ./bin/match.test --gtest_filter=TestCase.TestSetAndGet
TEST(TestCase, TestSetAndGet) {
    std::shared_ptr<RedisProxy::Response> resp;
    RedisProxy rp(REDIS_IP, PORT, CONNECTION_TIMEOUT_MS, RW_TIMEOUT_MS);
    const std::string key = "TestSetAndGet";
    bool is_succ;

    is_succ = rp.Set(key, "foo", 1);
    ASSERT_TRUE(is_succ);

    std::string value;
    is_succ = rp.Get(key, value);
    ASSERT_TRUE(is_succ);
    EXPECT_EQ("foo", value);
}

// =========================================Pipe Set Get=========================================
// ./bin/match.test --gtest_filter=TestCase.TestPipeSetAndGet
TEST(TestCase, TestPipeSetAndGet) {
    std::shared_ptr<RedisProxy::Response> resp;
    RedisProxy rp(REDIS_IP, PORT, CONNECTION_TIMEOUT_MS, RW_TIMEOUT_MS);
    const std::string key = "TestPipeSetAndGet";
    std::vector<std::string> keys;
    std::vector<std::string> values;
    std::vector<unsigned int> expire_time_secs;
    for (size_t i = 0; i < 100; i++) {
        keys.push_back(key + std::to_string(i));
        values.push_back(std::to_string(i));
        expire_time_secs.push_back(1);
    }

    bool is_succ;
    is_succ = rp.PipeSet(keys, values, expire_time_secs);
    ASSERT_TRUE(is_succ);

    std::vector<std::pair<bool, std::string>> values_recv;
    is_succ = rp.PipeGet(keys, values_recv);
    ASSERT_TRUE(is_succ);
    EXPECT_EQ(100, values_recv.size());
    for (size_t i = 0; i < 100; i++) {
        auto value = values_recv[i];
        EXPECT_EQ(true, value.first);
        EXPECT_EQ(std::to_string(i), value.second);
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}