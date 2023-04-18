#include "http/http_server/http_server.h"
#include "json/json.h"
#include "logger/logger.h"

void echo(http_server::HttpRequest* const req, http_server::HttpResponse* const resp) {
  std::string name = req->url_params["name"];
  Json::Value root;
  root["name"] = name;
  resp->body = root.toStyledString();
}

/**
$curl "127.0.0.1:8888/echo?name=tomocat"
{
   "name" : "tomocat"
}
 */
int main() {
  http_server::HttpServer http_server(8888);
  http_server.RegisterHandler("/echo", echo);
  http_server.Start();
  return 0;
}
