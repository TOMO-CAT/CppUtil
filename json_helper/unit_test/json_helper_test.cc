#include <mutex>

#include "gtest/gtest.h"
#include "json_helper/json_helper.h"

namespace json_helper {

TEST(JsonHelperTest, test_basic_type) {
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

}  // namespace json_helper