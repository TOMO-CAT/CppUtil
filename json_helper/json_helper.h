#pragma once

#include "json/json.h"
#include "json_helper/unmarshal.h"

namespace json_helper {

template <typename T>
bool Unmarshal(const std::string& str, T* const obj) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(str, root)) {
        return false;
    }
}

#define JSON_HELPER(...) JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(__VA_ARGS__)

}  // namespace json_helper