#include <string>

#include "http/http_client/http_client.h"

int main() {
  std::string resp;
  http_client::Get("www.baidu.com", 400, 200, &resp);
  printf("%s\n", resp.c_str());
}
