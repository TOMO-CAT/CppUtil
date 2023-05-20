cmake_minimum_required(VERSION 3.16)

project(CppUtil VERSION 1.0.0)

# Generate compile_commands.json
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Common compile options
add_definitions("-g")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "-lbacktrace")

# Install prefix
# set(CMAKE_INSTALL_PREFIX "")

# Include path
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cpptoml/include
)

# SourceFiles

## 1. Logger
file(GLOB_RECURSE logger_srcs logger/*.cc logger/*.cpp)
list(FILTER logger_srcs EXCLUDE REGEX ".*_test\.(cpp|cc)$")
MESSAGE(STATUS "this logger_srcs key = ${logger_srcs}.")

# Library
add_library(cpputil ${logger_srcs})

# Example
add_executable(
    logger_example
    logger/test/logger_test.cpp
    ${logger_srcs}
)
target_link_libraries(logger_example uuid backtrace)

# Install
install(
    TARGETS cpputil
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY logger
        DESTINATION include/cpputil
        FILES_MATCHING
        PATTERN "*.h"
        # PATTERN "*~" EXCLUDE
)

install(DIRECTORY util
        DESTINATION include/cpputil
        FILES_MATCHING
        PATTERN "*.h"
)
