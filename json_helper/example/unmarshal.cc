#include <cassert>
#include <mutex>
#include <string>

#include "json_helper/json_helper.h"

class Foo {
 private:
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

 private:
  // basic type
  int32_t money = 0;
  int64_t uid = 0;
  uint32_t age = 0;
  double birthday = 0;
  std::string name;

  // stl container
  std::vector<int> list;
  std::set<std::string> set;
  std::unordered_map<std::string, bool> map;

  // enum
  Color color;

  // enum class
  Gender gender;

  // nested class
  SubClass sub_class;

  // pointer
  int* pi;
  std::string* ps;
  SubClass* p_sub_class;

  // static variable
  static constexpr uint64_t id = 100;

  //  skip types
  std::mutex mtx_;

 public:
  JSON_HELPER(money, uid, age, birthday, name, list, set, color, gender, sub_class, map, pi, ps, p_sub_class, id);
};

int main() {
  std::string json_str = R"(
{
   "age" : 24,
   "birthday" : 3.1099999999999999,
   "color" : 1,
   "gender" : 1,
   "id" : 100,
   "list" : [ -5, 10, 100, 20033 ],
   "map" : {
      "dog" : false,
      "mouse" : true
   },
   "money" : -1,
   "name" : "foo",
   "p_sub_class" : {
      "name" : "p_sub_class",
      "pi" : 3.1415925025939941
   },
   "pi" : "nullptr",
   "ps" : "string",
   "set" : [ "cc", "kkk-mmm" ],
   "sub_class" : {
      "name" : "sub_class",
      "pi" : 3.1414999961853027
   },
   "uid" : 1234567890
}        
    )";

  Foo foo;
  printf("bbb\n");
  assert(0);
  printf("aaa\n");
}
