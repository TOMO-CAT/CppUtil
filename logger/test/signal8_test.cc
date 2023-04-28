#include <signal.h>

#include <iostream>

#include "logger/log.h"

int main() {
  LOG_INFO << "test crash with signal 8";
  ::raise(8);
}
