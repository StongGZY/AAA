#!/bin/bash

# 创建依赖目录
mkdir -p deps
mkdir -p third_party

# 设置安装路径
INSTALL_DIR=$(pwd)/deps/curl
BUILD_DIR=$(pwd)/third_party/curl

# 如果curl目录已存在，先删除
rm -rf $BUILD_DIR
rm -rf $INSTALL_DIR

# 下载并编译curl
cd third_party
git clone https://github.com/curl/curl.git
cd curl
git checkout curl-7_29_0

# 配置和编译
./buildconf
./configure --prefix=$INSTALL_DIR -without-ssl --without-libpsl --disable-ldap --disable-mqtt --disable-ipv6
make -j4
make install

echo "curl has been installed to $INSTALL_DIR" 