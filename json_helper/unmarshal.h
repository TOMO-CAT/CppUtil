#pragma once

#include <boost/preprocessor.hpp>               // BOOST_PP_VARIADIC_TO_SEQ
#include <boost/preprocessor/seq/for_each.hpp>  // BOOST_PP_SEQ_FOR_EACH
#include <boost/preprocessor/stringize.hpp>     // BOOST_PP_STRINGIZE
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "json/json.h"

namespace json_helper {

template <typename T, typename = std::void_t<>>
struct HasUnmarshalFunc : std::false_type {};
template <typename T>
struct HasUnmarshalFunc<T, std::void_t<decltype(&T::Unmarshal)>> : std::true_type {};

// return false for uncaptured types
template <typename T>
inline typename std::enable_if<!HasUnmarshalFunc<T>::value, bool>::type Unmarshal(const Json::Value& root,
                                                                                  T* const obj) {
    return false;
}

// class with Unmarshal function
template <typename T>
inline typename std::enable_if<HasUnmarshalFunc<T>::value, bool>::type Unmarshal(const Json::Value& root,
                                                                                 T* const obj) {
    return obj->Unmarshal(root);
}

// enum class

// int32_t
inline bool Unmarshal(const Json::Value& root, int32_t* const obj) {
    if (!root.isIntegral()) {
        return false;
    }
    *obj = root.asInt();
    return true;
}

// int64_t
inline bool Unmarshal(const Json::Value& root, int64_t* const obj) {
    if (!root.isIntegral()) {
        return false;
    }
    *obj = root.asInt64();
    return true;
}

// uint32_t
inline bool Unmarshal(const Json::Value& root, uint32_t* const obj) {
    if (!root.isIntegral()) {
        return false;
    }
    *obj = root.asUInt();
    return true;
}

// uint64_t
inline bool Unmarshal(const Json::Value& root, uint64_t* const obj) {
    if (!root.isIntegral()) {
        return false;
    }
    *obj = root.asUInt64();
    return true;
}

// float
inline bool Unmarshal(const Json::Value& root, float* const obj) {
    if (!root.isDouble()) {
        return false;
    }
    *obj = root.asFloat();
    return true;
}

// double
inline bool Unmarshal(const Json::Value& root, double* const obj) {
    if (!root.isDouble()) {
        return false;
    }
    *obj = root.asDouble();
    return true;
}

// bool
inline bool Unmarshal(const Json::Value& root, bool* const obj) {
    if (!root.isBool()) {
        return false;
    }
    *obj = root.asBool();
    return true;
}

// string
inline bool Unmarshal(const Json::Value& root, std::string* const obj) {
    if (!root.isString()) {
        return false;
    }
    *obj = root.asString();
    return true;
}

// std::vector
template <typename T>
bool Unmarshal(const Json::Value& root, std::vector<T>* const obj);

// std::map
template <typename T>
bool Unmarshal(const Json::Value& root, std::map<std::string, T>* const obj);
template <typename T>
bool Unmarshal(const Json::Value& root, std::map<int32_t, T>* const obj);
template <typename T>
bool Unmarshal(const Json::Value& root, std::map<uint32_t, T>* const obj);
template <typename T>
bool Unmarshal(const Json::Value& root, std::map<int64_t, T>* const obj);
template <typename T>
bool Unmarshal(const Json::Value& root, std::map<uint64_t, T>* const obj);

// std::unordered_map
template <typename T>
bool Unmarshal(const Json::Value& root, std::unordered_map<std::string, T>* const obj);
template <typename T>
bool Unmarshal(const Json::Value& root, std::unordered_map<int32_t, T>* const obj);
template <typename T>
bool Unmarshal(const Json::Value& root, std::unordered_map<uint32_t, T>* const obj);
template <typename T>
bool Unmarshal(const Json::Value& root, std::unordered_map<int64_t, T>* const obj);
template <typename T>
bool Unmarshal(const Json::Value& root, std::unordered_map<uint64_t, T>* const obj);

template <typename T>
inline bool Unmarshal(const Json::Value& root, std::vector<T>* const obj) {
    if (!root.isArray()) {
        return false;
    }
    obj->clear();
    obj->resize();
    bool ret = true;
    for (int i = 0; i < static_cast<int>(root.size()); ++i) {
        if (!Unmarshal(root[i], &obj[i])) {
            ret = false;
        }
    }
    return ret;
}

template <typename T>
bool Unmarshal(const Json::Value& root, std::map<std::string, T>* const obj) {
    if (!root.isObject()) {
        return false;
    }
    obj->clear();
    const auto& mems = root.getMemberNames();
    bool ret = true;
    for (auto iter = mems.begin(); iter != mems.end(); ++iter) {
        if (!Unmarshal(root[*iter], obj[*iter])) {
            ret = false;
        }
    }
    return ret;
}

template <typename T>
bool Unmarshal(const Json::Value& root, std::map<int32_t, T>* const obj) {
    if (!root.isObject()) {
        return false;
    }
    obj->clear();
    const auto& mems = root.getMemberNames();
    bool ret = true;
    for (auto iter = mems.begin(); iter != mems.end(); ++iter) {
        if (!Unmarshal(root[*iter], obj[std::stoi(*iter)])) {
            ret = false;
        }
    }
    return ret;
}

template <typename T>
bool Unmarshal(const Json::Value& root, std::map<uint32_t, T>* const obj) {
    if (!root.isObject()) {
        return false;
    }
    obj->clear();
    const auto& mems = root.getMemberNames();
    bool ret = true;
    for (auto iter = mems.begin(); iter != mems.end(); ++iter) {
        if (!Unmarshal(root[*iter], obj[std::stoul(*iter)])) {
            ret = false;
        }
    }
    return ret;
}

template <typename T>
bool Unmarshal(const Json::Value& root, std::map<int64_t, T>* const obj) {
    if (!root.isObject()) {
        return false;
    }
    obj->clear();
    const auto& mems = root.getMemberNames();
    bool ret = true;
    for (auto iter = mems.begin(); iter != mems.end(); ++iter) {
        if (!Unmarshal(root[*iter], obj[std::stoll(*iter)])) {
            ret = false;
        }
    }
    return ret;
}

template <typename T>
bool Unmarshal(const Json::Value& root, std::map<uint64_t, T>* const obj) {
    if (!root.isObject()) {
        return false;
    }
    obj->clear();
    const auto& mems = root.getMemberNames();
    bool ret = true;
    for (auto iter = mems.begin(); iter != mems.end(); ++iter) {
        if (!Unmarshal(root[*iter], obj[std::stoull(*iter)])) {
            ret = false;
        }
    }
    return ret;
}

template <typename T>
bool Unmarshal(const Json::Value& root, std::unordered_map<int32_t, T>* const obj) {
    if (!root.isObject()) {
        return false;
    }
    obj->clear();
    const auto& mems = root.getMemberNames();
    bool ret = true;
    for (auto iter = mems.begin(); iter != mems.end(); ++iter) {
        if (!Unmarshal(root[*iter], obj[std::stoi(*iter)])) {
            ret = false;
        }
    }
    return ret;
}

template <typename T>
bool Unmarshal(const Json::Value& root, std::unordered_map<uint32_t, T>* const obj) {
    if (!root.isObject()) {
        return false;
    }
    obj->clear();
    const auto& mems = root.getMemberNames();
    bool ret = true;
    for (auto iter = mems.begin(); iter != mems.end(); ++iter) {
        if (!Unmarshal(root[*iter], obj[std::stoul(*iter)])) {
            ret = false;
        }
    }
    return ret;
}

template <typename T>
bool Unmarshal(const Json::Value& root, std::unordered_map<int64_t, T>* const obj) {
    if (!root.isObject()) {
        return false;
    }
    obj->clear();
    const auto& mems = root.getMemberNames();
    bool ret = true;
    for (auto iter = mems.begin(); iter != mems.end(); ++iter) {
        if (!Unmarshal(root[*iter], obj[std::stoll(*iter)])) {
            ret = false;
        }
    }
    return ret;
}

template <typename T>
bool Unmarshal(const Json::Value& root, std::unordered_map<uint64_t, T>* const obj) {
    if (!root.isObject()) {
        return false;
    }
    obj->clear();
    const auto& mems = root.getMemberNames();
    bool ret = true;
    for (auto iter = mems.begin(); iter != mems.end(); ++iter) {
        if (!Unmarshal(root[*iter], obj[std::stoull(*iter)])) {
            ret = false;
        }
    }
    return ret;
}

#define __JSON_HELPER_UNMARSHAL_SINGLE_FIELD__(_1, _2, field)                                                                  \
    if (!json_helper::Unmarshal(root[BOOST_PP_STRINGIZE(field)], &field)) {                                                    \
        /*std::cout << "[json_helper][debug] parse " BOOST_PP_STRINGIZE(field) " fail" << std::endl; */                        \
        ret = false;                                                                                                           \
    } else {                                                                                                                   \
        /*std::cout << "[json_helper][debug] parse " BOOST_PP_STRINGIZE(field) " successfully, val: " << field << std::endl;*/ \
    }

#define JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(...)                                                               \
 public:                                                                                                         \
    bool Unmarshal(const Json::Value& root) {                                                                    \
        bool ret = true;                                                                                         \
        BOOST_PP_SEQ_FOR_EACH(__JSON_HELPER_UNMARSHAL_SINGLE_FIELD__, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)); \
        return ret;                                                                                              \
    }

}  // namespace json_helper