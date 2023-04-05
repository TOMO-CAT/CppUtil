#pragma once

#include "json/json.h"  // jsoncpp

/**
 * @brief 为成员函数
*/
#define JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(...) \
 public:                                           \
    bool Unmarshal(const std::string& str, T) {    \
        Json::Value root;                          \
        Json::Reader reader;                       \
        if (!reader.parse(str, root)) {            \
            return false;                          \
        }                                          \
        /*TODO*/                                   \
    }
