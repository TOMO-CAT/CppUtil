#include <memory>
#include "logger.h"
#include "cpptoml.h"

namespace logger {

Logger* Logger::instance_ = new Logger();

Logger::Logger() : is_console_output_(true), file_appender_(nullptr), priority_(Level::INFO_LEVEL) {}

Logger::~Logger() {
    if (file_appender_) {
        delete file_appender_;
    }
}

bool Logger::Init(const std::string& conf_path) {
    std::shared_ptr<cpptoml::table> g;
    try {
        g = cpptoml::parse_file(conf_path);
        
    }
}

}  // namespace logger