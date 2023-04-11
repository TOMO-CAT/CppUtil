#!/bin/bash

# 获取脚本所在的目录绝对路径
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# 基于 cpp_projects 镜像创建名为 cpp_util_container 的容器
docker run --name cpp_util_container -v ${script_dir}:/CppUtil -it cpp_util