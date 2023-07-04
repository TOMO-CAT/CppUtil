#include <iostream>

#include "logger/backtrace.h"

void fail() {
  for (int i = 0; i < 10; ++i) {
    std::cout << "==================== " << i << "====================" << std::endl;
    std::vector<std::string> stacks;
    logger::StackDumper(0).Dump(&stacks);
    for (auto&& s : stacks) {
      std::cout << s << std::endl;
    }
  }
}

void bar() {
  fail();
}

void foo() {
  bar();
}

int main() {
  foo();
}
