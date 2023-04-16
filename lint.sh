#!/bin/bash

# 蓝色 info 日志
function log_info() {
    printf "\e[34m\e[1m[INFO]\e[0m $*\n"
}

# 橙色 warn 日志
function log_warn() {
    printf "\033[0;33m[WARN]\e[0m $*\n"
}

# 红色 error 日志
function log_error() {
    printf "\033[0;31m[ERRO]\e[0m $*\n"
}

# 绿色 ok 日志
function log_ok() {
    printf "\e[32m\e[1m[ OK ]\e[0m $*\n"
}

# lint 单个目录
function lint_single_directory() {
    log_info "start lint $1"
    [ -d $1 ] || {
        log_error "directory '$1' don't exist"
        exit 1
    }
    res=`find $1 -name "*.h" -o -name "*.cc" -o -name "*.hpp" -o -name "*.cpp" | xargs cpplint`
    echo "$res" | grep -v "Done processing"    
}

CPPLINT_DIRECTORY_LIST="
./http_client
./http_server
./json_helper
./logger
./redis_proxy
./threadpool
"

function lint_all_directory() {
    for i in `echo "$CPPLINT_DIRECTORY_LIST"`
    do
        lint_single_directory $i
    done
}

if [ $# -ge 1 ]; then
    lint_single_directory "$1"
else
    lint_all_directory
fi

[ $? == 0 ] && log_ok