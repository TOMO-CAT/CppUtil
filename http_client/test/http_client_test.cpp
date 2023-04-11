#include <string>

#include "http_client/http_client.h"

int main() {
    std::string resp;
    httpclient::Get("www.baidu.com", 400, 200, resp);
    printf("%s", resp.c_str());
}