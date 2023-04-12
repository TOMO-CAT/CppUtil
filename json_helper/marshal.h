#pragma once

#include <boost/preprocessor.hpp>               // BOOST_PP_VARIADIC_TO_SEQ
#include <boost/preprocessor/seq/for_each.hpp>  // BOOST_PP_SEQ_FOR_EACH
#include <boost/preprocessor/stringize.hpp>     // BOOST_PP_STRINGIZE
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "json/config.h"
#include "json/json.h"
#include "json_helper/util.h"

namespace json_helper {

// return false for uncaptured types
template <typename T>
typename std::enable_if<!HasMarshalFunc<T>::value && !IsEnumClass<T>::value, bool>::type Marshal(
    const T& obj, Json::Value* const root);

// class with Marshal function
template <typename T>
typename std::enable_if<HasMarshalFunc<T>::value && !IsEnumClass<T>::value, bool>::type Marshal(
    const T& obj, Json::Value* const root);

// enum class
template <typename T>
typename std::enable_if<IsEnumClass<T>::value && !HasMarshalFunc<T>::value, bool>::type Marshal(
    const T& obj, Json::Value* const root);

// basic types
bool Marshal(const int32_t obj, Json::Value* const root);
bool Marshal(const int64_t obj, Json::Value* const root);
bool Marshal(const uint32_t obj, Json::Value* const root);
bool Marshal(const uint64_t obj, Json::Value* const root);
bool Marshal(const float obj, Json::Value* const root);
bool Marshal(const double obj, Json::Value* const root);
bool Marshal(const bool obj, Json::Value* const root);
bool Marshal(const std::string& obj, Json::Value* const root);

// std::vector<T>
template <typename T>
bool Marshal(const std::vector<T>& obj, Json::Value* const root);

// std::map<K, V>
template <typename V>
bool Marshal(const std::map<std::string, V>& obj, Json::Value* const root);
template <typename K, typename V>
bool Marshal(const std::map<K, V>& obj, Json::Value* const root);

// std::unordered_map<K, V>
template <typename V>
bool Marshal(const std::unordered_map<std::string, V>& obj, Json::Value* const root);
template <typename K, typename V>
bool Marshal(const std::unordered_map<K, V>& obj, Json::Value* const root);

// std::set<T>
template <typename T>
bool Marshal(const std::set<T>& obj, Json::Value* const root);

// std::unordered_set<T>
template <typename T>
bool Marshal(const std::unordered_set<T>& obj, Json::Value* const root);

#define __JSON_HELPER_MARSHAL_SINGLE_FIELD__(_1, _2, field)                    \
    if (!json_helper::Marshal(field, &((*root)[BOOST_PP_STRINGIZE(field)]))) { \
        ret = false;                                                           \
    }

#define JSON_HELPER_MARSHAL_MEMBER_FUNCTION(...)                                                               \
    bool Marshal(Json::Value* root) {                                                                          \
        bool ret = true;                                                                                       \
        BOOST_PP_SEQ_FOR_EACH(__JSON_HELPER_MARSHAL_SINGLE_FIELD__, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)); \
        return ret;                                                                                            \
    }

// ==============================================  Implementation ==============================================
// uncaptured types
template <typename T>
typename std::enable_if<!HasMarshalFunc<T>::value && !IsEnumClass<T>::value, bool>::type Marshal(
    const T& obj, Json::Value* const root) {
    if (_JSON_HELPER_DEBUG) {
        std::cout << "[JsonHelper][Marshal][Warning] fallback to uncaptured types: " << typeid(obj).name() << std::endl;
    }
    return false;
}

// class with Marshal function
template <typename T>
typename std::enable_if<HasMarshalFunc<T>::value && !IsEnumClass<T>::value, bool>::type Marshal(
    const T& obj, Json::Value* const root) {
    return obj.Marshal(root);
}

// enum class
template <typename T>
typename std::enable_if<IsEnumClass<T>::value && !HasMarshalFunc<T>::value, bool>::type Marshal(
    const T& obj, Json::Value* const root) {
    *root = Json::Int(static_cast<int>(obj));
    return true;
}

// int32_t
inline bool Marshal(const int32_t obj, Json::Value* const root) {
    *root = Json::Int(obj);
    return true;
}

// int64_t
inline bool Marshal(const int64_t obj, Json::Value* const root) {
    *root = Json::Int64(obj);
    return true;
}

// uint32_t
inline bool Marshal(const uint32_t obj, Json::Value* const root) {
    *root = Json::UInt(obj);
    return true;
}

// uint64_t
inline bool Marshal(const uint64_t obj, Json::Value* const root) {
    *root = Json::UInt64(obj);
    return true;
}

// float
inline bool Marshal(const float obj, Json::Value* const root) {
    *root = obj;
    return true;
}

// double
inline bool Marshal(const double obj, Json::Value* const root) {
    *root = obj;
    return true;
}

// bool
inline bool Marshal(const bool obj, Json::Value* const root) {
    *root = obj;
    return true;
}

// std::string
inline bool Marshal(const std::string& obj, Json::Value* const root) {
    *root = obj;
    return true;
}

// std::vector<T>
template <typename T>
bool Marshal(const std::vector<T>& obj, Json::Value* const root) {
    bool ret = true;
    for (int i = 0; i < static_cast<int>(obj.size()); ++i) {
        if (!Marshal(obj[i], &((*root)[i]))) {
            ret = false;
        }
    }
    return ret;
}

// std::map<std::string, V>
template <typename V>
bool Marshal(const std::map<std::string, V>& obj, Json::Value* const root) {
    bool ret = true;
    for (auto iter = obj.begin(); iter != obj.end(); ++iter) {
        if (!Marshal(iter->second, &((*root)[iter->first]))) {
            ret = false;
        }
    }
    return ret;
}

// std::map<K, V>
template <typename K, typename V>
bool Marshal(const std::map<K, V>& obj, Json::Value* const root) {
    bool ret = true;
    for (auto iter = obj.begin(); iter != obj.end(); ++iter) {
        if (!Marshal(iter->second, &((*root)[std::to_string(iter->first)]))) {
            ret = false;
        }
    }
    return ret;
}

// std::unordered_map<std::string, V>
template <typename V>
bool Marshal(const std::unordered_map<std::string, V>& obj, Json::Value* const root) {
    bool ret = true;
    for (auto iter = obj.begin(); iter != obj.end(); ++iter) {
        if (!Marshal(iter->second, &((*root)[iter->first]))) {
            ret = false;
        }
    }
    return ret;
}

// std::unordered_map<K, V>
template <typename K, typename V>
bool Marshal(const std::unordered_map<K, V>& obj, Json::Value* const root) {
    bool ret = true;
    for (auto iter = obj.begin(); iter != obj.end(); ++iter) {
        if (!Marshal(iter->second, &((*root)[std::to_string(iter->first)]))) {
            ret = false;
        }
    }
    return ret;
}

// std::set<T>
template <typename T>
bool Marshal(const std::set<T>& obj, Json::Value* const root) {
    bool ret = true;
    for (auto iter = obj.begin(); iter != obj.end(); ++iter) {
        Json::Value elem;
        if (!Marshal(*iter, &elem)) {
            ret = false;
        } else {
            root->append(elem);
        }
    }
    return ret;
}

// std::unordered_set<T>
template <typename T>
bool Marshal(const std::unordered_set<T>& obj, Json::Value* const root) {
    bool ret = true;
    for (auto iter = obj.begin(); iter != obj.end(); ++iter) {
        Json::Value elem;
        if (!Marshal(*iter, &elem)) {
            ret = false;
        } else {
            root->append(elem);
        }
    }
    return ret;
}

}  // namespace json_helper