#include <signal.h>

#include "logger/log.h"

int main() {
  LOG_INFO << "info message";
  LOG_ERROR << "error message";

  ::raise(8);
}
