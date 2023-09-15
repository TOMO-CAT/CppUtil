#include <cassert>
#include <mutex>
#include <string>
#include <unordered_map>

#include "json_helper/json_helper.h"
#include "logger/log.h"

class Foo {
 public:
  struct SubClass {
    std::string name;
    float pi = 0.0;

    JSON_HELPER(name, pi);
  };

  enum Color {
    kUnknown = 0,
    kBlue = 1,
    kRead = 2,
    kBlack = 3,
  };

  enum class Gender {
    kUnknown = 0,
    kMale = 1,
    kFemale = 2,
  };

 public:
  // basic type
  int32_t money = 0;
  int64_t uid = 0;
  uint32_t age = 0;
  double birthday = 0;
  std::string name;

  // stl container
  std::vector<int> list;
  std::set<std::string> set;
  // std::unordered_map<std::string, bool> map;
  std::unordered_map<int, int> map;

  // enum
  Color color;

  // enum class
  Gender gender;

  // nested class
  SubClass sub_class;

  // static variable
  static constexpr uint64_t id = 100;

  // pointer
  int* pi = nullptr;
  std::string* ps = nullptr;
  SubClass* p_sub_class = nullptr;

  //  skip types
  std::mutex mtx_;

 public:
  JSON_HELPER(money, uid, age, birthday, name, list, set, color, gender, sub_class, map, pi, ps, p_sub_class);
};

int main() {
  std::string json_str = R"(
{
  "age": 24,
  "birthday": 3.1099999999999999,
  "color": 1,
  "gender": 1,
  "list": [-5, 10, 100, 20033],
  "map": {
    "10": 20,
    "30": 123
  },
  "money": -1,
  "name": "foo",
  "p_sub_class": {
    "name": "p_sub_class",
    "pi": 3.1415925025939941
  },
  "pi": "nullptr",
  "ps": "string",
  "set": ["cc", "kkk-mmm"],
  "sub_class": {
    "name": "sub_class",
    "pi": 3.1414999961853027
  },
  "uid": 1234567890
}
    )";

  Foo foo;

  // 读取成 ::Json::Value 再转成 String
  // 保证是一个合法 Json
  {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json_str, root)) {
      std::cout << "jsoncpp parse fail!" << std::endl;
      return -1;
    }
    Json::StyledWriter writer;
    std::cout << "======================= jsoncpp =======================" << std::endl;
    std::cout << "jsoncpp str:" << writer.write(root) << std::endl;
    std::cout << "=======================================================" << std::endl;
  }

  // 通过 json_helper 进行反序列化和序列化
  {
    std::cout << "===================== json_helper =====================" << std::endl;
    CHECK(::json_helper::Unmarshal(json_str, &foo));
    std::cout << ::json_helper::ToStringFormatted(foo) << std::endl;
    std::cout << "=======================================================" << std::endl;

    CHECK_EQ(24, foo.age);
    CHECK_EQ(Foo::Color::kBlue, foo.color);
    CHECK_EQ(Foo::Gender::kMale, foo.gender);
  }
}
