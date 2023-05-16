#include "json_helper/json_helper.h"

int main() {
  class Cat {
   private:
    std::string name = "cc";
    int age = -10;
    double birthday = 3.1;
    std::vector<int> favorite_nums = {5, 7, 9};

   public:
    JSON_HELPER_MARSHAL_MEMBER_FUNCTION(name, age, birthday, favorite_nums);
  };
  std::cout << json_helper::ToString(Cat()) << std::endl;
}
