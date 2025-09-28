#!/bin/bash

#sudo apt-get install autoconf-archive
#sudo apt-get install autoconf automake libtool

current_path=$(pwd)

# 定义环境变量
export TOOLCHAIN_DIR="$HOME/sdk/BSP_RK3506/prebuilts/gcc/linux-x86/arm/gcc-arm-10.3-2021.07-x86_64-arm-none-linux-gnueabihf/bin"
export TARGET_PLATFORM="arm-none-linux-gnueabihf"
export CROSS_COMPILE="${TARGET_PLATFORM}-"
export CC="${CROSS_COMPILE}gcc"
export CXX="${CROSS_COMPILE}g++"
export PATH="$TOOLCHAIN_DIR:$PATH"

export INSTALL_PATH=$current_path/build/libgpiod-2.1/install

# 创建安装目录
tar -xf linuxc_prj/third_party/libgpiod-2.1.tar.gz -C build
mkdir -p ${INSTALL_PATH}
cd $current_path/build/libgpiod-2.1

./autogen.sh  # 生成 configure 脚本
./configure \
  --host=${TARGET_PLATFORM} \
  --prefix=${INSTALL_PATH} \
  --enable-static=no \
  --enable-tests=no

make -j$(nproc)
make install
