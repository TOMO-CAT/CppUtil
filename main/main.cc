#include <iostream>

#include "util/magic_enum.h"

enum Color { RED, GREEN, BLUE };

int main() {
  Color color = RED;
  std::cout << util::enum_name(color) << std::endl;
  // output => RED
}
