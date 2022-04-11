#include "httpclient.h"

int main() {
    httpclient::CHttpClient http_client;
    std::string resp;
    http_client.Get("www.baidu.com", resp);
    printf("resp: %s", resp.c_str());
}