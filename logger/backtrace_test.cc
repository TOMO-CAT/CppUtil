#include "logger/backtrace.h"

#include <iostream>

void fail() {
  std::vector<std::string> stacks;
  logger::StackDumper(0).Dump(&stacks);
  for (auto&& s : stacks) {
    std::cout << s << std::endl;
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
