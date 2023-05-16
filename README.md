# CppUtil

## 简介

C++ 常用库。

## docker

```bash
$bash docker.sh
```

## 编译

### 1. xmake

```bash
# 编译全部目标
$xmake -a

# 安装到 pkg
# $xmake install -o pkg

# 安装到 /usr/local
$xmake install -o /usr/local
```

### 2. blade

```bash
# 编译 logger
$bear blade build logger/...

# 编译 json helper
$bear blade build json_helper/...
```

## 待续

1. docker.sh 脚本支持当前 user
