#include <iostream>
#include <vector>

#include "json/json.h"

template <typename T>
inline bool Unmarshal(const Json::Value& root, std::vector<T>* const obj) {
    std::cout << "[####### debug #######]" << std::endl;
    if (!root.isArray()) {
        std::cout << "no array" << std::endl;
        return false;
    }
    obj->clear();
    obj->resize(root.size());
    bool ret = true;
    for (int i = 0; i < static_cast<int>(root.size()); ++i) {
        if (!Unmarshal(root[i], &obj[i])) {
            std::cout << "i: " << i << ", root[i]: " << root[i] << std::endl;
            ret = false;
        }
    }
    std::cout << "[####### done #######]" << std::endl;
    return ret;
}

int main() {
    std::string str = R"(
        ["a", "b", "c"]
    )";

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(str, root)) {
        std::cout << "parse fail" << std::endl;
        exit(-1);
    }
    if (!root.isArray()) {
        std::cout << "root is not array" << std::endl;
    }

    std::vector<std::string> res;
    Unmarshal(root, &res);
}