#include "util/magic_enum.h"

#include "gtest/gtest.h"

namespace util {

enum Color { RED, GREEN, BLUE };

TEST(UtilTest, magic_enum_test) {
  EXPECT_EQ("util::RED", enum_name(Color::RED));
  EXPECT_EQ("util::GREEN", enum_name(Color::GREEN));
  EXPECT_EQ("util::BLUE", enum_name(Color::BLUE));
}

}  // namespace util
