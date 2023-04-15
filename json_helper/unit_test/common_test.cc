#include <mutex>

#include "gtest/gtest.h"
#include "json_helper/json_helper.h"

namespace json_helper {

// HasUnmarshalFunc<T> 模板类用于辅助判断类 T 是否定义了 Unmarshal 方法
TEST(CommonTest, test_HasUnmarshalFunc) {
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

// HasMarshalFunc<T> 模板类用于辅助判断类 T 是否定义了 Marshal 方法
TEST(CommonTest, test_HasMarshalFunc) {
  struct Class_WithMarshalMemberFunction {
    bool Marshal(Json::Value* const root) {
      return true;
    }
  };

  EXPECT_TRUE(::json_helper::HasMarshalFunc<Class_WithMarshalMemberFunction>::value);

  struct Class_WithoutMarshalMemberFunction {};
  EXPECT_FALSE(::json_helper::HasMarshalFunc<Class_WithoutMarshalMemberFunction>::value);

  struct Cat_WithMarshalMacro {
    std::string name;
    int32_t age;
    double birthday;

    JSON_HELPER_MARSHAL_MEMBER_FUNCTION(name, age, birthday);
  };
  EXPECT_TRUE(::json_helper::HasMarshalFunc<Cat_WithMarshalMacro>::value);
}

// std::is_enum<T> 模版类用于判断类 T 是否是 enum
TEST(CommonTest, test_std_is_enum) {
  enum Color {
    kUnknown = 0,
    kRead = 1,
    kBlue = 2,
  };
  EXPECT_TRUE(std::is_enum<Color>::value);

  enum class Gender {
    kUnknown = 0,
    kMale = 1,
    kFemale = 2,
  };
  EXPECT_TRUE(std::is_enum<Gender>::value);
}

}  // namespace json_helper
