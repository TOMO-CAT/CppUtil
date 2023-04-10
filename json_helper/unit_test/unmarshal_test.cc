#include <mutex>

#include "gtest/gtest.h"
#include "json_helper/unmarshal.h"

namespace json_helper {

// 反序列化基础类型
TEST(UnmarshalTest, unmarshal_basic_type) {
    const std::string str = R"(
    {
        "name": "tomo",
        "age": 12,
        "birthday": 5.11,
        "pi": 3.1415926
    }
    )";

    Json::Value root;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(str, root));

    std::string res_str;
    int32_t res_int32_t = 0;
    float res_float = 0.0;
    double res_double = 0.0;

    ASSERT_TRUE(::json_helper::Unmarshal(root["name"], &res_str));
    ASSERT_TRUE(::json_helper::Unmarshal(root["age"], &res_int32_t));
    ASSERT_TRUE(::json_helper::Unmarshal(root["birthday"], &res_float));
    ASSERT_TRUE(::json_helper::Unmarshal(root["pi"], &res_double));

    EXPECT_EQ("tomo", res_str);
    EXPECT_EQ(12, res_int32_t);
    EXPECT_FLOAT_EQ(5.11, res_float);
    EXPECT_DOUBLE_EQ(3.1415926, res_double);
}

// 反序列化非嵌套的类
TEST(UnmarshalTest, unmarshal_plain_class) {
    struct Cat {
        std::string name;
        int age = -1;
        double birthday = 0.0;

        JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(name, age, birthday);
    };

    const std::string str = R"(
    {
        "name": "tomo",
        "age": 12,
        "birthday": 5.11
    }
    )";
    Json::Value root;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(str, root));

    // 1. 直接调用宏生成的 Unmarshal 成员函数
    Cat cat;
    ASSERT_TRUE(cat.Unmarshal(root));
    EXPECT_EQ("tomo", cat.name);
    EXPECT_EQ(12, cat.age);
    EXPECT_DOUBLE_EQ(5.11, cat.birthday);

    // 2. 使用重载的 Unmarshal 函数
    Cat cat2;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &cat2));
    EXPECT_EQ("tomo", cat2.name);
    EXPECT_EQ(12, cat2.age);
    EXPECT_DOUBLE_EQ(5.11, cat2.birthday);
}

// 对于未特例化的类型调用 Unmarshal 方法返回 false
TEST(UnmarshalTest, test_uncaptured_types) {
    // 1. class without Unmarshal member function
    struct Dog {
        std::string name;
        int age = -1;
    };

    const std::string str = R"(
    {
        "name": "tomo",
        "age": 12
    }
    )";

    Json::Value root;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(str, root));
    Dog dog;
    ASSERT_FALSE(::json_helper::Unmarshal(root, &dog));
    EXPECT_EQ("", dog.name);
    EXPECT_EQ(-1, dog.age);

    // 2. std::mutex
    std::mutex mtx;
    ASSERT_FALSE(::json_helper::Unmarshal(root, &mtx));
}

// HasUnmarshalFunc<T> 模板类用于辅助判断类 T 是否定义了 Unmarshal 方法
TEST(UnmarshalTest, test_HasUnmarshalFunc) {
    struct Class_WithUnmarshalMemberFunction {
        bool Unmarshal(const Json::Value& root) {
            return true;
        }
    };
    EXPECT_TRUE(::json_helper::HasUnmarshalFunc<Class_WithUnmarshalMemberFunction>::value);

    struct Class_WithoutUnmarshalMemberFunction {};
    EXPECT_FALSE(::json_helper::HasUnmarshalFunc<Class_WithoutUnmarshalMemberFunction>::value);

    struct Cat_WithUnmarshalMacro {
        std::string name;
        int32_t age;
        double birthday;

        JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(name, age, birthday);
    };
    EXPECT_TRUE(::json_helper::HasUnmarshalFunc<Cat_WithUnmarshalMacro>::value);
}

// enum 类的 Unmarshal
TEST(UnmarshalTest, test_enum_class) {
    enum class Color {
        kUnknown = 0,
        kRead = 1,
        kBlue = 2,
        kBlack = 3,
    };

    std::unordered_map<uint32_t, Color> num2enum = {
        {0, Color::kUnknown},
        {1, Color::kRead},
        {2, Color::kBlue},
        {3, Color::kBlack},
    };

    Json::Value root;
    for (uint32_t i = 0; i < num2enum.size(); ++i) {
        root[std::to_string(i)] = i;
    }

    Color color;
    for (uint32_t i = 0; i < num2enum.size(); ++i) {
        ::json_helper::Unmarshal(root[std::to_string(i)], &color);
        std::cout << "color: " << static_cast<uint32_t>(color) << std::endl;
        EXPECT_EQ(num2enum[i], color);
    }
}

// 嵌套类的 Unmarshal
TEST(UnmarshalTest, test_nested_class) {
    class Vehicle {
        class Wheel {
            int temp = 0;
            double pressure = 0.0;
            std::string factory;

            JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(temp, pressure, factory);
        };

        Wheel wheel;
        std::string color;

        JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(wheel, color);
    };
}

}  // namespace json_helper