#include <string>
#include <iostream>
#include <cstdio>
#include "jsoncpp/json.h"

void Hello(const std::string& req, Json::Value& root) {
    root["hello"] = "world";
}

int main() {
    Json::Value root;

    root["name"] = "TOMOCAT";
    root["age"] = 33;
    Json::Value cities;
    cities.append("BeiJing");
    cities.append("ShangHai");
    cities.append("GuangZhou");
    root["cities"] = cities;

    /* 序列化为Json */
    std::string strRoot = root.toStyledString();
    std::cout << strRoot << std::endl;

    return 0;
}