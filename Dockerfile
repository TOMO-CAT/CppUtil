# 指定基础镜像
FROM ubuntu:22.04

# 安装依赖
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list \
    && apt-get clean && apt-get update && apt-get install -y --fix-missing \
    build-essential \
    git \
    python2.7 \
    ninja-build \
    bear \
    wget \
    uuid-dev \
    unzip \
    # libuv1-dev \
    # libglib2.0-dev \
    # libsystemd-dev \
    # libev-dev \
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

# 安装 curl
RUN wget https://curl.se/download/archeology/curl-7.29.0.tar.gz --no-check-certificate -P /curl && \
    cd /curl && \
    tar xvf curl-7.29.0.tar.gz && \
    cd curl-7.29.0 \
    ./configure && \
    make && \
    make install && \
    echo 'export LD_LIBRARY_PATH=/usr/local/lib' >> ~/.bashrc && \
    bash

# 安装 xmake
# RUN wget https://xmake.io/shget.text -O - | bash && /bin/bash -c "source ~/.xmake/profile"
RUN wget https://xmake.io/shget.text -O - | bash && echo 'source ~/.xmake/profile' >> ~/.bashrc
ENV  XMAKE_ROOT=y

# 指定工作目录
WORKDIR /CppUtil
