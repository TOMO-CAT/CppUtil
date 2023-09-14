#include "json_helper/json_helper.h"

#include <mutex>

#include "gtest/gtest.h"

namespace json_helper {

TEST(JsonHelperTest, unmarshal_test_basic_type) {
  int32_t i32 = {};
  int64_t i64 = {};
  uint32_t ui32 = {};
  uint64_t ui64 = {};
  std::string str = {};
  float fl = {};
  double db = {};

  ASSERT_TRUE(::json_helper::Unmarshal("123", &i32));
  EXPECT_EQ(123, i32);
  ASSERT_TRUE(::json_helper::Unmarshal("321", &i64));
  EXPECT_EQ(321, i64);
  ASSERT_TRUE(::json_helper::Unmarshal("1111", &ui32));
  EXPECT_EQ(1111u, ui32);
  ASSERT_TRUE(::json_helper::Unmarshal("2222", &ui64));
  EXPECT_EQ(2222u, ui64);
  ASSERT_TRUE(::json_helper::Unmarshal(R"("kkkk")", &str));
  EXPECT_EQ("kkkk", str);
  ASSERT_TRUE(::json_helper::Unmarshal("3.14", &fl));
  EXPECT_FLOAT_EQ(3.14, fl);
  ASSERT_TRUE(::json_helper::Unmarshal("9.666", &db));
  EXPECT_DOUBLE_EQ(9.666, db);
}

TEST(JsonHelperTest, to_string) {
  {
    // plain class
    class Cat {
     private:
      std::string name = "cc";
      int age = -10;
      double birthday = 3.1;
      std::vector<int> favorite_nums = {5, 7, 9};

     public:
      JSON_HELPER_MARSHAL_MEMBER_FUNCTION(name, age, birthday, favorite_nums);
    };

    Cat cat;
    std::string expected_str = R"({"age":-10,"birthday":3.1000000000000001,"favorite_nums":[5,7,9],"name":"cc"})";
    EXPECT_EQ(expected_str, ::json_helper::ToString(cat));
  }
}

}  // namespace json_helper
