#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "json_helper/json_helper.h"

namespace {
std::string g_str = "global_string";
}

class Foo {
 private:
    struct SubClass {
        std::string name = "sub_class";
        float pi = 3.1415;
    };

 private:
    // basic type
    int32_t money = -1;
    int64_t uid = 1234567890;
    uint32_t age = 24;
    double birthday = 3.11;
    std::string name = "foo";

    // stl container
    std::vector<int> list = {-5, 10, 100, 20033};
    std::set<std::string> set = {"cc", "kkk-mmm"};
    std::unordered_map<std::string, bool> map = {
        {"mouse", true},
        {"dog", false},
    };

    // nested class
    SubClass sub_class;

    // pointer
    int* pi = nullptr;
    std::string* ps = &g_str;
    SubClass* p_sub_class = new SubClass{
        .name = "p_sub_class",
        .pi = 3.1415926,
    };

    // static variable
    static constexpr uint64_t id = 100;

    //  skip types
    std::mutex mtx_;

 public:
    JSON_HELPER(money, uid, age, birthday, name, list, set, sub_class, map, pi, ps, p_sub_class, id);
};

int main() {
    Foo foo;
    std::string json_str;
    ::json_helper::Marshal(foo, &json_str, true);
    std::cout << json_str << std::endl;

    return 0;
}