#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "json_helper/json_helper.h"

class Foo {
 private:
  struct SubClass {
    std::string name = "sub_class";
    float pi = 3.1415;

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

 private:
  // basic type
  int32_t money = -1;
  int64_t uid = 1234567890;
  uint32_t age = 24;
  double birthday = 3.11;
  std::string name = "foo";

  // stl container
  std::vector<int> list = {-5, 10, 100, 20033};
  std::set<std::string> set = {"cc", "kkk-mmm"};
  std::unordered_map<std::string, bool> map = {
      {"mouse", true},
      {"dog", false},
  };

  // enum
  Color color = kBlue;

  // enum class
  Gender gender = Gender::kMale;

  // nested class
  SubClass sub_class;

  // pointer
  int* pi = nullptr;
  std::string* ps = new std::string("string");
  SubClass* p_sub_class = new SubClass{
      .name = "p_sub_class",
      .pi = 3.1415926,
  };

  // static variable
  static constexpr uint64_t id = 100;

  //  skip types
  std::mutex mtx_;

 public:
  JSON_HELPER(money, uid, age, birthday, name, list, set, color, gender, sub_class, map, pi, ps, p_sub_class, id);
};

int main() {
  Foo foo;
  std::string json_str;
  ::json_helper::Marshal(foo, &json_str, true);
  std::cout << "============ with style ============" << std::endl;
  std::cout << json_str << std::endl;

  std::cout << "=========== without style ==========" << std::endl;
  ::json_helper::Marshal(foo, &json_str);
  std::cout << json_str << std::endl;
  return 0;
}
