# httpclient

## 簡介

简易Http客戶端。

## 特性
* 支持设置连接超时和传输超时
* 支持设置debug模式（设置宏_HTTP_CLIENT_DEBUG为true）
* 支持CA证书
* 支持POST和GET

## 例子
```c++
#include "httpclient.h"

int main() {
    std::string resp;
    httpclient::Get("www.baidu.com", 400, 200, resp);
    printf("%s", resp.c_str());
}
```