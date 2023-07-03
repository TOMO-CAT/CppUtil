#include <filesystem>

// #include "logger/log.h"

struct Foo {
  int32_t i32 = -10;
  uint64_t ui64 = 1234;
  double db = 3.1415926;
  bool bl = true;
  std::string str = "cat";
  const char* ch = "dog";
};

#include <iostream>

int main() {
  std::string str = "mmm";
  std::cout << str << std::endl;
  [](std::string* const s) {
    *s = "cccc";
  }(&str);
  std::cout << str << std::endl;

  // // 格式化字符串日志
  // LogInfo("info message");
  // LogDebug("debug message");
  // LogInfo("name:%s age:%d weight:%.1f", "tomocat", 26, 56.23);
  // LogWarn("warn message");
  // LogError("error message");
  // LogErrorWithTag("err_tag", "error message with tag, type:%s length:%d", "pencil", 17);

  // // 流式日志
  // LOG_INFO << "double: " << 3.14 << ", int64_t:" << -801;
  // LOG_WARN << "warn message";
  // LOG_ERROR << "error message";

  // // KV 日志
  // Foo foo;
  // LogInfoKV("example_prefix")
  //     .LogKV("int32_data", foo.i32)
  //     .LogKV("uint64_data", foo.ui64)
  //     .LogKV("double_data", foo.db)
  //     .LogKV("bool_data", foo.bl)
  //     .LogKV("string_data", foo.str)
  //     .LogKV("char_data", foo.ch)
  //     .LogKV("float_format_data=%.2f", foo.db);

  // // Fatal 日志
  // LogFatal("x must be larger than 0!");

  // // 断言
  // CHECK(false);
  // CHECK_EQ(3, 4);
  // CHECK_NOTNULL(nullptr);
}
