#include "logger/logger.h"

int main() {
    if (!logger::Logger::GetInstance()->Init("./conf/logger.conf")) {
        log_error("init logger fail, print to console");
    }

    log_info("info message");
    log_debug("debug message");
    log_info("name:%s age:%d weight:%.1f", "tomocat", 26, 56.23);
    log_warn("warn message");
    log_error("error message");
    log_error_t("err_tag", "error message with tag, type:%s length:%d", "pencil", 17);
}