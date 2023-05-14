#include <signal.h>

#include "logger/log.h"

int main() {
  for (int i = 0; i < 10000; ++i) {
    LOG_INFO_FIRST_N(5) << "info message";
  }
}
