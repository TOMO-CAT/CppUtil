#include "http_server.h"

using httpserver::Request;
using httpserver::Response;

// void Hello(Request& req, Json::Value& root) {
//     root["hello"] = "world";
// }
// void SayHello(Request& req, Json::Value& root) {
//     root["name"] = req.get_param("name");
//     root["age"] = atoi(req.get_param("age").c_str());
// }

void Sleep(Request& req, Response& root) {
}

int main(int argc, char** args) {
    if (argc < 2) {
        log_error("usage: ./http_server_test [port]");
        return -1;
    }

    httpserver::HttpServer http_server;
    // http_server.AddMapping("/hello", Hello);
    // http_server.AddMapping("/sayhello", SayHello);
    http_server.AddMapping("/sleep", Sleep);

    int port = atoi(args[1]);
    int backlog = 100000;
    int max_events = 1000;

    http_server.Start(port, backlog, max_events);
    return 0;
}