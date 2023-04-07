#include <iostream>

#include "json_helper/unmarshal.h"
struct Cat {
    enum class Color {
        kWhite = 0,
        kBlack = 1,
        kBlue = 2,
        kOther = 3,
    };

    std::string name;
    int32_t age;

    bool Unmarshal(const Json::Value& root) {
        return true;
    }
};

int main() {
    Cat cat = {
        .name = "John",
        .age = 10,
    };

    std::cout << json_helper::HasUnmarshalFunc<Cat>::value << std::endl;

    return 0;
}