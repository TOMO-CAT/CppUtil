#include "json_helper/marshal.h"

#include <mutex>
#include <string>
#include <vector>

#include "gtest/gtest.h"

namespace json_helper {

TEST(MarshalTest, marshal_basic_types) {
  int32_t i32 = -123;
  int64_t i64 = -32123;
  uint32_t ui32 = 777;
  uint64_t ui64 = 888;
  float fl = 3.14;
  double db = -7.12;
  bool bl = true;
  std::string str = "kkkk";

  Json::Value root;
  ASSERT_TRUE(::json_helper::Marshal(i32, &root));
  ASSERT_EQ(-123, root.asInt());

  ASSERT_TRUE(::json_helper::Marshal(i64, &root));
  ASSERT_EQ(-32123, root.asInt64());

  ASSERT_TRUE(::json_helper::Marshal(ui32, &root));
  ASSERT_EQ(777u, root.asUInt());

  ASSERT_TRUE(::json_helper::Marshal(ui64, &root));
  ASSERT_EQ(888u, root.asUInt64());

  ASSERT_TRUE(::json_helper::Marshal(fl, &root));
  ASSERT_FLOAT_EQ(3.14, root.asFloat());

  ASSERT_TRUE(::json_helper::Marshal(db, &root));
  ASSERT_DOUBLE_EQ(-7.12, root.asDouble());

  ASSERT_TRUE(::json_helper::Marshal(bl, &root));
  ASSERT_TRUE(root.asBool());

  ASSERT_TRUE(::json_helper::Marshal(str, &root));
  ASSERT_EQ("kkkk", root.asString());
}

TEST(MarshalTest, marshal_vector) {
  std::vector<int32_t> vi = {1, 22, 333, 4444};
  Json::Value root;

  ASSERT_TRUE(::json_helper::Marshal(vi, &root));
  ASSERT_EQ(vi.size(), root.size());
  for (int i = 0; i < static_cast<int>(vi.size()); ++i) {
    EXPECT_EQ(vi[i], root[i].asInt());
  }
}

TEST(MarshalTest, marshal_map) {
  Json::Value root;

  std::map<std::string, int32_t> m_str_i32 = {
      {"kkk", 10},
      {"ccc", -5},
      {"mmm", -100},
  };
  ASSERT_TRUE(::json_helper::Marshal(m_str_i32, &root));
  ASSERT_EQ(m_str_i32.size(), root.size());
  EXPECT_EQ(m_str_i32["kkk"], root["kkk"].asInt());
  EXPECT_EQ(m_str_i32["ccc"], root["ccc"].asInt());
  EXPECT_EQ(m_str_i32["mmm"], root["mmm"].asInt());
  root.clear();

  std::map<uint64_t, std::string> m_ui64_str = {
      {111, "1"},
      {222, "22"},
      {333, "333"},
  };
  ASSERT_TRUE(::json_helper::Marshal(m_ui64_str, &root));
  ASSERT_EQ(m_ui64_str.size(), root.size());
  EXPECT_EQ(m_ui64_str[111], root["111"].asString());
  EXPECT_EQ(m_ui64_str[222], root["222"].asString());
  EXPECT_EQ(m_ui64_str[333], root["333"].asString());
  root.clear();
}

TEST(MarshalTest, marshal_unordered_map) {
  Json::Value root;

  std::unordered_map<std::string, int32_t> m_str_i32 = {
      {"kkk", 10},
      {"ccc", -5},
      {"mmm", -100},
  };
  ASSERT_TRUE(::json_helper::Marshal(m_str_i32, &root));
  ASSERT_EQ(m_str_i32.size(), root.size());
  EXPECT_EQ(m_str_i32["kkk"], root["kkk"].asInt());
  EXPECT_EQ(m_str_i32["ccc"], root["ccc"].asInt());
  EXPECT_EQ(m_str_i32["mmm"], root["mmm"].asInt());
  root.clear();

  std::unordered_map<uint64_t, std::string> m_ui64_str = {
      {111, "1"},
      {222, "22"},
      {333, "333"},
  };
  ASSERT_TRUE(::json_helper::Marshal(m_ui64_str, &root));
  ASSERT_EQ(m_ui64_str.size(), root.size());
  EXPECT_EQ(m_ui64_str[111], root["111"].asString());
  EXPECT_EQ(m_ui64_str[222], root["222"].asString());
  EXPECT_EQ(m_ui64_str[333], root["333"].asString());
  root.clear();
}

TEST(MarshalTest, marshal_set_int32) {
  Json::Value root;
  std::set<int32_t> s = {1, -5, 20, 100, 7};
  ASSERT_TRUE(::json_helper::Marshal(s, &root));

  ASSERT_EQ(s.size(), root.size());
  EXPECT_EQ(-5, root[0].asInt());
  EXPECT_EQ(1, root[1].asInt());
  EXPECT_EQ(7, root[2].asInt());
  EXPECT_EQ(20, root[3].asInt());
  EXPECT_EQ(100, root[4].asInt());
}

TEST(MarshalTest, marshal_unordered_set_string) {
  Json::Value root;
  std::unordered_set<std::string> us = {"111", "aaa", "to"};
  ASSERT_TRUE(::json_helper::Marshal(us, &root));

  ASSERT_EQ(us.size(), root.size());

  for (auto iter = us.begin(); iter != us.end(); ++iter) {
    bool exist = false;
    for (int i = 0; i < static_cast<int>(root.size()); ++i) {
      if (root[i].asString() == *iter) {
        exist = true;
        break;
      }
    }
    ASSERT_TRUE(exist);
  }
}

TEST(MarshalTest, marshal_plain_class) {
  class Cat {
   private:
    std::string name = "cc";
    int age = -10;
    double birthday = 3.1;
    std::vector<int> favorite_nums = {5, 7, 9};

   public:
    JSON_HELPER_MARSHAL_MEMBER_FUNCTION(name, age, birthday, favorite_nums);
  };

  Json::Value root;
  Cat cat;
  cat.Marshal(&root);

  Json::FastWriter writer;
  std::string expected_str = R"({"age":-10,"birthday":3.1000000000000001,"favorite_nums":[5,7,9],"name":"cc"})";
  std::string actual_str = writer.write(root);
  EXPECT_EQ(expected_str + "\n", actual_str);
}

TEST(MarshalTest, marshal_uncaptured_types) {
  Json::Value root;

  // 1. class without marshal member function
  struct Dog {
    std::string name;
  };
  Dog dog;
  ASSERT_FALSE(::json_helper::Marshal(dog, &root));

  // 2. std::mutex
  std::mutex mutex;
  ASSERT_FALSE(::json_helper::Marshal(mutex, &root));
}

TEST(MarshalTest, marshal_enum_calss) {
  enum class Color {
    kUnknown = 0,
    kRead = 1,
    kBlue = 2,
    kBlack = 3,
  };

  Json::Value root;

  for (int i = 0; i < 4; ++i) {
    Color color = static_cast<Color>(i);
    ASSERT_TRUE(::json_helper::Marshal(color, &root));
    EXPECT_EQ(i, root.asInt());
  }
}

TEST(MarshalTest, marshal_nested_class) {
  class Vehicle {
   public:
    class Wheel {
     public:
      int temp = 10;
      double pressure = 3.2;
      std::string factory = "Audi";

      JSON_HELPER_MARSHAL_MEMBER_FUNCTION(temp, pressure, factory);
    };

    Wheel wheel;
    std::string color = "blue";

    JSON_HELPER_MARSHAL_MEMBER_FUNCTION(wheel, color);
  };

  Json::Value root;
  Vehicle vehicle;
  ASSERT_TRUE(vehicle.Marshal(&root));

  Json::FastWriter writer;
  std::string expected_str = R"({"color":"blue","wheel":{"factory":"Audi","pressure":3.2000000000000002,"temp":10}})";
  std::string actual_str = writer.write(root);
  EXPECT_EQ(expected_str + "\n", actual_str);
}

TEST(MarshalTest, marshal_pointer) {
  Json::Value root;

  {
    int* pi = nullptr;
    ASSERT_TRUE(::json_helper::Marshal(pi, &root));
    EXPECT_EQ("nullptr", root.asString());
  }
  {
    int i = 10;
    int* pi = &i;
    ASSERT_TRUE(::json_helper::Marshal(pi, &root));
    EXPECT_EQ(10, root.asInt());
  }
}

}  // namespace json_helper