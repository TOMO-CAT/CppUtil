#!/bin/bash

set -eu

# 项目所在文件夹
PROJECT_BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")";pwd)"
# ClangFormat 二进制名, 例如 clang-format-8 clang-format-13 或 clang-format
CLANG_FORMAT_BIN="clang-format"

function main() {
  # 检查参数合法性
  [ $# -lt 1 ] && {
    echo "-----------------------------------------"
    echo "Error: Please specify a path to format."
    echo "-----------------------------------------"
    echo "Usage: bash clang-format.sh TO_FORMAT_DIR"
    echo "-----------------------------------------"
    exit -1
  }

  # 检查是否有 clang-format 可执行程序
  if command -v "${CLANG_FORMAT_BIN}" >/dev/null 2>&1; then
    echo "-----------------------------------------"
    echo "Info: ${CLANG_FORMAT_BIN} bin file exist"
    echo "-----------------------------------------"
  else
    echo "-----------------------------------------"
    echo "Error: ${CLANG_FORMAT_BIN} bin file doesn't exist, quit!"
    echo "-----------------------------------------"
    exit -1
  fi

  # 执行格式化
  local to_format_dir=$1
  echo "-----------------------------------------"
  echo "Info: Going to format directory '${to_format_dir}' ..."
  find "${to_format_dir}" -name "*.h" \
    -o -name "*.cc" \
    -o -name "*.hpp" \
    -o -name "*.cpp" \
    -o -name "*.proto" | xargs ${CLANG_FORMAT_BIN} -style=file -i --verbose
  echo "Info: done!"
  echo "-----------------------------------------"
}

main $*
