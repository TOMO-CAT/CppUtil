#pragma once

#include "json/json.h"
#include "json_helper/unmarshal.h"

namespace json_helper {

template <typename T>
bool Unmarshal(const std::string& str, T* const obj) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(str, root)) {
        if (_JSON_HELPER_DEBUG) {
            std::cout << "[JsonHelper][Warning] jsoncpp parse fail, str: " << str << std::endl;
        }
        return false;
    }
    return Unmarshal(root, obj);
}

template <typename T>
bool Unmarshal(const char* str, T* const obj) {
    return Unmarshal(std::string(str), obj);
}

#define JSON_HELPER(...) JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(__VA_ARGS__)

}  // namespace json_helper