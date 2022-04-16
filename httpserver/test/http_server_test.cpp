#include "http_server.h"
#include "logger.h"
#include "jsoncpp/json.h"

void echo(httpserver::HttpRequest& req, httpserver::HttpResponse& resp) {
    std::string name = req.headers["name"];
    Json::Value root;
    root["name"] = name;
    resp.body = root.toStyledString();
}

int main() {
    httpserver::HttpServer http_server(8888);
    http_server.RegisterHandler("/echo", echo);

    http_server.Start();
    return 0;
}