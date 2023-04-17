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
  * 格式化控制符：`LogInfo("%s is %d years old.", "Lili", 8);`
  * 流式：`LOG_INFO << "Lily is " << 8 << " years old.";`
  * KV 日志：`LogInfoKV("student info").LogKV("name", "lily").LogKV("age", 8);`
* 日志信息丰富，包括时间、线程号、UUID、日志级别、文件、行号、函数名
* 支持断言，断言失败时打印堆栈并退出程序

## 具体使用方法

编译：

```bash
# makefile
$cd logger
$make

# blade
# 1. release 模式
$blade build logger
# 2. debug 模式
$blade build -pdebug logger

# 使用 bear 编译
$bear -- blade build logger
```

测试：

```bash
$cd logger
$make test
$./output/bin/TestLogger 
[2022-04-17 17:21:28.885058][3271:c4e0d7b4e734cac][INFO ][../util/config_util/toml_helper.h:23][ParseTomlValue]parse toml succ, key:Level value:1
[2022-04-17 17:21:28.885121][3271:c4e0d7b4e734cac][INFO ][../util/config_util/toml_helper.h:23][ParseTomlValue]parse toml succ, key:Directory value:./log
[2022-04-17 17:21:28.885166][3271:c4e0d7b4e734cac][INFO ][../util/config_util/toml_helper.h:23][ParseTomlValue]parse toml succ, key:FileName value:logger.log
[2022-04-17 17:21:28.885198][3271:c4e0d7b4e734cac][INFO ][../util/config_util/toml_helper.h:23][ParseTomlValue]parse toml succ, key:RetainHours value:4
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
