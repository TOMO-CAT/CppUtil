#include "httpclient.h"

int main() {
    std::string resp;
    int ret = httpclient::Get("www.baidu.com", 400, 200, resp);
    printf("%s", resp.c_str());
}