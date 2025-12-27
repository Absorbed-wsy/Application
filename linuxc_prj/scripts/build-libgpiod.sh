#!/bin/bash

#sudo apt-get install autoconf-archive
#sudo apt-get install autoconf automake libtool

current_path=$(pwd)

# 定义环境变量
export TOOLCHAIN_DIR="/home/wsy/SDK/Toolchain/aarch64-rk-linux_gcc-10.3.1/bin"
export TARGET_PLATFORM="aarch64-none-linux-gnu"
export CROSS_COMPILE="${TARGET_PLATFORM}-"
export CC="${CROSS_COMPILE}gcc"
export CXX="${CROSS_COMPILE}g++"
export PATH="$TOOLCHAIN_DIR:$PATH"

export INSTALL_PATH=/home/wsy/SDK/Toolchain/aarch64-rk-linux_gcc-10.3.1/aarch64-none-linux-gnu/libc

# 创建安装目录
tar -xf ./third_party/libgpiod-1.6.x.tar.gz -C build
mkdir -p ${INSTALL_PATH}
cd ./build/libgpiod-1.6.x

./autogen.sh  # 生成 configure 脚本
./configure \
  --host=${TARGET_PLATFORM} \
  --prefix=${INSTALL_PATH} \
  --enable-static=no \
  --enable-tests=no

make -j$(nproc)
make install
