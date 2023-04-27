#include "logger/backtrace.h"

#include <iostream>

#include "backtrace.h"
#include "gtest/gtest.h"

TEST(TestBacktrace, print_stack) {
  std::cout << logger::StackDumper(0).Dump() << std::endl;
}
