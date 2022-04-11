#include "httpclient.h"

int main() {
    std::string resp;
    httpclient::Get("www.baidu.com", resp);
    printf("%s", resp.c_str());
}