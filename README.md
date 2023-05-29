# CppUtil

## 简介

C++ 常用库。

## docker

```bash
$bash docker.sh
```

## 编译

### 1. cmake

```bash
# 创建编译文件夹
$ mkdir build
$ cd build

# 编译全部目标
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make

# 执行 logger 测试代码
$ ./bin/logger_example

# 安装
# 1. 安装到 ./output 目录
# $ cmake --install . --prefix output
# 2. 安装到 /usr/local 目录
$ cmake --install .
```

### 2. xmake

```bash
# 编译全部目标
$xmake -a

# 安装到 pkg
# $xmake install -o pkg

# 安装到 /usr/local
$xmake install -o /usr/local
```

### 3. blade

```bash
# 编译 logger
$bear blade build logger/...

# 编译 json helper
$bear blade build json_helper/...
```

## 待续

1. docker.sh 脚本支持当前 user
