#include "logger/log.h"

int main() {
  if (!logger::Logger::Instance()->Init("./logger.conf")) {
    LOG_ERROR << "Init logger fail, print to console";
  }

  logger::Logger::set_trace_id();
  LOG_INFO << "Init logger successfully!";
}
