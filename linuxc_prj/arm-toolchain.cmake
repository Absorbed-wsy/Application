# arm-toolchain.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 指定交叉编译器路径 / 设置库搜索路径（根据实际路径调整）
#RK3506
#set(CMAKE_C_COMPILER /home/wsy/sdk/BSP_RK3506/prebuilts/gcc/linux-x86/arm/gcc-arm-10.3-2021.07-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-gcc)
#set(CMAKE_CXX_COMPILER /home/wsy/sdk/BSP_RK3506/prebuilts/gcc/linux-x86/arm/gcc-arm-10.3-2021.07-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-g++)
#set(CMAKE_FIND_ROOT_PATH /home/wsy/sdk/BSP_RK3506/prebuilts/gcc/linux-x86/arm/gcc-arm-10.3-2021.07-x86_64-arm-none-linux-gnueabihf/arm-none-linux-gnueabihf/libc)

#S5P6818
#set(CMAKE_C_COMPILER /opt/FriendlyARM/toolchain/6.4-aarch64/bin/aarch64-linux-gcc)
#set(CMAKE_CXX_COMPILER /opt/FriendlyARM/toolchain/6.4-aarch64/bin/aarch64-linux-g++)
#set(CMAKE_FIND_ROOT_PATH /opt/FriendlyARM/toolchain/6.4-aarch64/aarch64-cortexa53-linux-gnu/sysroot)

# 搜索策略：只在目标环境查找库
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
