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

// IsEnumClass<T> 模版类用于辅助判断类 T 是否是 enum class
TEST(UnmarshalTest, test_IsEnumClass) {
    struct Cat {
        std::string name;
        int32_t age = -1;
    };

    enum class Color { Red, Green, Blue };

    EXPECT_FALSE(::json_helper::IsEnumClass<Cat>::value);
    EXPECT_TRUE(::json_helper::IsEnumClass<Color>::value);
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
        EXPECT_EQ(num2enum[i], color);
    }
}

// 嵌套类的 Unmarshal
TEST(UnmarshalTest, test_nested_class) {
    class Vehicle {
     public:
        class Wheel {
         public:
            int temp = 0;
            double pressure = 0.0;
            std::string factory;

            JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(temp, pressure, factory);
        };

        Wheel wheel;
        std::string color;

        JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(wheel, color);
    };

    std::string str = R"(
        {
            "wheel": {
                "temp": 45,
                "pressure": 85.5,
                "factory": "Audi"
            },
            "color": "blue"
        }
    )";

    Json::Value root;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(str, root));

    Vehicle vehicle;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &vehicle));

    EXPECT_EQ("blue", vehicle.color);
    EXPECT_EQ(45, vehicle.wheel.temp);
    EXPECT_DOUBLE_EQ(85.5, vehicle.wheel.pressure);
    EXPECT_EQ("Audi", vehicle.wheel.factory);
}

TEST(UnmarshalTest, test_unmarshal_vector) {
    std::string str = R"(
        ["a", "b", "c"]
    )";

    Json::Value root;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(str, root));
    ASSERT_TRUE(root.isArray());

    std::vector<std::string> res;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &res));

    ASSERT_EQ(3u, res.size());
    EXPECT_EQ("a", res[0]);
    EXPECT_EQ("b", res[1]);
    EXPECT_EQ("c", res[2]);
}

TEST(UnmarshalTest, test_unmarshal_map_string_key) {
    std::string m = R"(
        {
            "cat": 10,
            "dog": 1107,
            "mouse": 2133
        }
    )";

    Json::Value root;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(m, root));
    ASSERT_TRUE(root.isObject());

    std::map<std::string, int32_t> res;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &res));

    ASSERT_EQ(3u, res.size());
    EXPECT_EQ(10, res["cat"]);
    EXPECT_EQ(1107, res["dog"]);
    EXPECT_EQ(2133, res["mouse"]);
}

TEST(UnmarshalTest, test_unmarshal_map_int_key) {
    std::string m = R"(
        {
            "1": 10,
            "2": 1107,
            "3": 2133
        }
    )";

    Json::Value root;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(m, root));
    ASSERT_TRUE(root.isObject());

    // int32_t
    std::map<int32_t, int32_t> int32_key_res;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &int32_key_res));

    ASSERT_EQ(3u, int32_key_res.size());
    EXPECT_EQ(10, int32_key_res[1]);
    EXPECT_EQ(1107, int32_key_res[2]);
    EXPECT_EQ(2133, int32_key_res[3]);

    // int64_t
    std::map<int64_t, int32_t> int64_key_res;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &int64_key_res));

    ASSERT_EQ(3u, int64_key_res.size());
    EXPECT_EQ(10, int64_key_res[1]);
    EXPECT_EQ(1107, int64_key_res[2]);
    EXPECT_EQ(2133, int64_key_res[3]);

    // uint32_t
    std::map<int64_t, int32_t> uint32_key_res;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &uint32_key_res));

    ASSERT_EQ(3u, uint32_key_res.size());
    EXPECT_EQ(10, uint32_key_res[1]);
    EXPECT_EQ(1107, uint32_key_res[2]);
    EXPECT_EQ(2133, uint32_key_res[3]);

    // uint64_t
    std::map<int64_t, int32_t> uint64_key_res;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &uint64_key_res));

    ASSERT_EQ(3u, uint64_key_res.size());
    EXPECT_EQ(10, uint64_key_res[1]);
    EXPECT_EQ(1107, uint64_key_res[2]);
    EXPECT_EQ(2133, uint64_key_res[3]);
}

TEST(UnmarshalTest, test_unmarshal_unordered_map_int_key) {
    std::string m = R"(
        {
            "1": 115,
            "2": 107,
            "3": 233
        }
    )";

    Json::Value root;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(m, root));
    ASSERT_TRUE(root.isObject());

    // int32_t
    std::unordered_map<int32_t, int32_t> int32_key_res;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &int32_key_res));

    ASSERT_EQ(3u, int32_key_res.size());
    EXPECT_EQ(115, int32_key_res[1]);
    EXPECT_EQ(107, int32_key_res[2]);
    EXPECT_EQ(233, int32_key_res[3]);

    // int64_t
    std::unordered_map<int64_t, int32_t> int64_key_res;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &int64_key_res));

    ASSERT_EQ(3u, int64_key_res.size());
    EXPECT_EQ(115, int64_key_res[1]);
    EXPECT_EQ(107, int64_key_res[2]);
    EXPECT_EQ(233, int64_key_res[3]);

    // uint32_t
    std::unordered_map<int64_t, int32_t> uint32_key_res;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &uint32_key_res));

    ASSERT_EQ(3u, uint32_key_res.size());
    EXPECT_EQ(115, uint32_key_res[1]);
    EXPECT_EQ(107, uint32_key_res[2]);
    EXPECT_EQ(233, uint32_key_res[3]);

    // uint64_t
    std::unordered_map<int64_t, int32_t> uint64_key_res;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &uint64_key_res));

    ASSERT_EQ(3u, uint64_key_res.size());
    EXPECT_EQ(115, uint64_key_res[1]);
    EXPECT_EQ(107, uint64_key_res[2]);
    EXPECT_EQ(233, uint64_key_res[3]);
}

TEST(UnmarshalTest, test_unmarshal_set_and_unordered_set) {
    std::string m = R"(
        [
            "cat",
            "dog",
            "mouse"
        ]
    )";

    Json::Value root;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(m, root));
    ASSERT_TRUE(root.isArray());

    // set
    std::set<std::string> s;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &s));
    ASSERT_EQ(3u, s.size());
    EXPECT_EQ(1u, s.count("cat"));
    EXPECT_EQ(1u, s.count("dog"));
    EXPECT_EQ(1u, s.count("mouse"));

    // unordered_set
    std::unordered_set<std::string> us;
    ASSERT_TRUE(::json_helper::Unmarshal(root, &us));
    ASSERT_EQ(3u, us.size());
    EXPECT_EQ(1u, us.count("cat"));
    EXPECT_EQ(1u, us.count("dog"));
    EXPECT_EQ(1u, us.count("mouse"));
}

TEST(UnmarshalTest, unmarshal_pointer) {
    Json::Value root;
    root = Json::Int(40);

    // 反序列化到普通的 int 类型
    {
        int i = 0;
        ASSERT_TRUE(::json_helper::Unmarshal(root, &i));
        EXPECT_EQ(40, i);
    }

    // 反序列化到 int 类型: 使用指针
    {
        int i = 0;
        int* pi = &i;
        ASSERT_TRUE(::json_helper::Unmarshal(root, pi));
        EXPECT_EQ(40, i);
    }

    // 反序列化到 int 类型: nullptr
    {
        int* pi = nullptr;
        EXPECT_DEATH(::json_helper::Unmarshal(root, pi), "");
        EXPECT_EQ(nullptr, pi);
    }

    // 反序列化到 int* 类型
    {
        // 非 nullptr
        {
            int i = 0;
            int* pi = &i;
            ASSERT_TRUE(::json_helper::Unmarshal(root, &pi));
            EXPECT_EQ(40, i);
        }
        // nullptr
        {
            int* pi = nullptr;
            EXPECT_FALSE(::json_helper::Unmarshal(root, &pi));
        }
    }
}

}  // namespace json_helper