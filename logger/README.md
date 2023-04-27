# 日志库

## 简介

线程安全的 C++ 同步日志库。

## 特性

* 默认输出到控制台
* 支持配置日志保存路径和文件
* 每小时自动切割日志
* 支持设置日志最大保存时长，自动清理过期日志
* 支持 DEBUG、INFO、WARN、ERROR 和 FATAL 五种级别日志输出，FATAL 日志触发时打印堆栈并退出程序
* 支持多种日志形式
  * 格式化控制符：`LogInfo("%s is %d years old.", "Lily", 8);`
  * 流式：`LOG_INFO << "Lily is " << 8 << " years old.";`
  * KV 日志：`LogInfoKV("student info").LogKV("name", "lily").LogKV("age", 8);`
* 日志信息丰富，包括时间、线程号、UUID、日志级别、文件、行号、函数名
* 支持断言，断言失败时打印堆栈并退出程序

## 具体使用方法

编译：

```bash
# release 模式
$blade build logger/...

# debug 模式
$blade build -pdebug logger/...

# 使用 bear 生成 compile_commands.json 方便调试
$bear -- blade build logger/...
```

测试：

```bash
$./build64_debug/logger/test/logger_test
[2023-04-23 10:37:07.963554][0:e24eaba10b72ea75][ERROR][logger/test/logger_test.cpp:26][main] error message
[2023-04-23 10:37:07.963579][0:e24eaba10b72ea75][ERROR][logger/test/logger_test.cpp:27][main][tag=err_tag] error message with tag, type:pencil length:17
[2023-04-23 10:37:07.963601][0:e24eaba10b72ea75][ERROR][logger/test/logger_test.cpp:32][main] error message
[2023-04-23 10:37:07.963610][0:e24eaba10b72ea75][FATAL][logger/test/logger_test.cpp:46][main] x must be larger than 0!
        Exiting due to FATAL log
        Call Stack:
                ./build64_debug/logger/test/logger_test(+0xb5c8) [0x55c12fc235c8]
                ./build64_debug/logger/test/logger_test(+0x8dd3) [0x55c12fc20dd3]
                /lib/x86_64-linux-gnu/libc.so.6(+0x29d90) [0x7ffb451fad90]
                /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0x80) [0x7ffb451fae40]
                ./build64_debug/logger/test/logger_test(+0x81e5) [0x55c12fc201e5]
[2023-04-23 10:37:07.964395][0:e24eaba10b72ea75][FATAL][logger/test/logger_test.cpp:49][main] 
        CHECK(false) fail.
        Exiting due to FATAL log
        Call Stack:
                ./build64_debug/logger/test/logger_test(+0xb5c8) [0x55c12fc235c8]
                ./build64_debug/logger/test/logger_test(+0xa70b) [0x55c12fc2270b]
                ./build64_debug/logger/test/logger_test(+0x8eb2) [0x55c12fc20eb2]
                /lib/x86_64-linux-gnu/libc.so.6(+0x29d90) [0x7ffb451fad90]
                /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0x80) [0x7ffb451fae40]
                ./build64_debug/logger/test/logger_test(+0x81e5) [0x55c12fc201e5]
[2023-04-23 10:37:07.964489][0:e24eaba10b72ea75][FATAL][logger/test/logger_test.cpp:50][main] 
        CHECK(3 == 4) fail.
        Exiting due to FATAL log
        Call Stack:
                ./build64_debug/logger/test/logger_test(+0xb5c8) [0x55c12fc235c8]
                ./build64_debug/logger/test/logger_test(+0xa70b) [0x55c12fc2270b]
                ./build64_debug/logger/test/logger_test(+0x902b) [0x55c12fc2102b]
                /lib/x86_64-linux-gnu/libc.so.6(+0x29d90) [0x7ffb451fad90]
                /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0x80) [0x7ffb451fae40]
                ./build64_debug/logger/test/logger_test(+0x81e5) [0x55c12fc201e5]
[2023-04-23 10:37:07.964548][0:e24eaba10b72ea75][FATAL][logger/test/logger_test.cpp:51][main] 
        CHECK(nullptr != nullptr) fail.
        Exiting due to FATAL log
        Call Stack:
                ./build64_debug/logger/test/logger_test(+0xb5c8) [0x55c12fc235c8]
                ./build64_debug/logger/test/logger_test(+0xa70b) [0x55c12fc2270b]
                ./build64_debug/logger/test/logger_test(+0x91a2) [0x55c12fc211a2]
                /lib/x86_64-linux-gnu/libc.so.6(+0x29d90) [0x7ffb451fad90]
                /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0x80) [0x7ffb451fae40]
                ./build64_debug/logger/test/logger_test(+0x81e5) [0x55c12fc201e5]
```

测试代码：

```c++
#include "logger.h"

int main() {
    if (!logger::Logger::GetInstance()->Init("./conf/logger.conf")) {
        log_error("init logger fail, print to console");
    }

    log_info("info message");
    log_debug("debug message");
    log_info("name:%s age:%d weight:%.1f", "tomocat", 26, 56.23);
    log_warn("warn message");
    log_error("error message");
    log_error_t("err_tag", "error message with tag, type:%s length:%d", "pencil", 17);
}
```

配置文件：

```toml
# 日志级别, 默认打印INFO日志
#   * 0: DEBUG
#   * 1: INFO
#   * 2: WARN
#   * 3: ERROR
Level=1
# 日志存储文件夹, 默认输出到当前文件夹
Directory="./log"
# 日志文件名, 默认输出到控制台
FileName="logger.log"
# 保存小时数, 不设置则不会进行日志切割
RetainHours=4
```

## TODO

* 作为配置项支持异步写入日志
* 性能测试
* 支持自定义日志格式
* 收到信号时打印堆栈并退出
* 解除对 util 的依赖并提供 make install 脚本
