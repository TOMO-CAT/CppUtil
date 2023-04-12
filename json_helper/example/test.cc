#include <iostream>

#include "json/json.h"

int main() {
    // 创建一个 Json::Value 对象
    Json::Value root;
    root["name"] = "Alice";
    root["age"] = 20;
    root["gender"] = "female";

    // 使用 FastWriter 将 Json::Value 对象序列化成字符串
    Json::FastWriter fastWriter;
    std::string jsonString1 = fastWriter.write(root);

    // 使用 StyledWriter 将 Json::Value 对象序列化成字符串
    Json::StyledWriter styledWriter;
    std::string jsonString2 = styledWriter.write(root);

    // 输出结果
    std::cout << "FastWriter: " << jsonString1 << std::endl;
    std::cout << "StyledWriter: " << jsonString2 << std::endl;

    return 0;
}