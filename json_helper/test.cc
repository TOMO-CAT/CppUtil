// #include <iostream>

// #include "json_helper/unmarshal.h"
// struct Cat {
//     enum class Color {
//         kWhite = 0,
//         kBlack = 1,
//         kBlue = 2,
//         kOther = 3,
//     };

//     std::string name;
//     int32_t age;

//     bool Unmarshal(const Json::Value& root) {
//         return true;
//     }
// };

// int main() {
//     Cat cat = {
//         .name = "John",
//         .age = 10,
//     };

//     std::cout << json_helper::HasUnmarshalFunc<Cat>::value << std::endl;

//     return 0;
// }

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <iostream>

#define FIELDS (name)(age)(address)

class Person {
 public:
    std::string name;
    int age;
    std::string address;
};

#define PRINT_FIELD(r, data, field) std::cout << BOOST_PP_STRINGIZE(field) << " = " << data.field << '\n';

#define PRINT_FIELDS(obj) BOOST_PP_SEQ_FOR_EACH(PRINT_FIELD, obj, FIELDS)

int main() {
    Person p{"Alice", 25, "123 Main St."};
    PRINT_FIELDS(p)
    return 0;
}