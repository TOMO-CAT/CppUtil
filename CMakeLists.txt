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

# Include path
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Source files
set(logger_srcs logger/*.cc)
MESSAGE(STATUS "this logger_srcs key = ${logger_srcs}.")
