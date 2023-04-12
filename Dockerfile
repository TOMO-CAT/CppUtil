# 指定基础镜像
FROM ubuntu:22.04

# 安装依赖
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list \
    && apt-get clean && apt-get update && apt-get install -y --fix-missing \
    build-essential \
    git \
    curl \
    python2.7 \
    ninja-build \
    bear \
    uuid-dev \
    # 如果需要在 docker 中使用 apt 的话需要注掉这一行
    # && rm -rf /var/lib/apt/lists/* \
    && ln -s /usr/bin/python2.7 /usr/bin/python


# 安装 blade
RUN git clone https://github.com/chen3feng/blade-build.git --branch v2.0 --single-branch --depth=1 /blade-build && \
    cd /blade-build && \
    ./install && \
    # 单引号以纯字符串的方式写入 ~/.bashrc, 而不会被解释为变量或者命令
    echo 'export PATH=~/bin:$PATH' >> ~/.bashrc && \
    # cd .. && rm -rf /blade-build && \
    bash

# 指定工作目录
WORKDIR /CppUtil