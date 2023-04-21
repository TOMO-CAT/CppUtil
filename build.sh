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

if [ $# -ge 1 ]; then
    # blade dump --compdb --to-file compile_commands.json "$1"
    bear blade build "$1"
else
    log_error "missing parameters that need to be built"
fi

log_ok
