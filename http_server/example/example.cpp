#include "http_server/http_server.h"
#include "json/json.h"
#include "logger/logger.h"

void echo(httpserver::HttpRequest* const req, httpserver::HttpResponse* const resp) {
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
  httpserver::HttpServer http_server(8888);
  http_server.RegisterHandler("/echo", echo);

  http_server.Start();
  return 0;
}
