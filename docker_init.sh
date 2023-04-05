#!/bin/bash

set -e

# 当前脚本所在路径, 即项目根路径
ROOT_PATH=$(cd "$(dirname "$0")";pwd)

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

function main() {
    # 参数数量
    PARAM_CNT=$#
    # 第一个参数表示运行环境: offline / online
    RUNNING_ENV=$1

    sudo chown -R $USER /$HOME/.docker || sudo chmod -R g+rwx "/$HOME/.docker" || echo 'run docker with root...'
    log_info "info"
    log_warn "warn"
    log_error "error"
    log_ok "ok"
}

main "$@"