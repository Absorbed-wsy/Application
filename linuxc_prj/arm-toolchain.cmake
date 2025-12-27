# arm-toolchain.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)

# 指定交叉编译器路径 / 设置库搜索路径（根据实际路径调整）
#RK3399
#set(TOOLCHAIN_DIR "${PROJECT_ROOT}/../BSP_RK3399/prebuilts/gcc/linux-x86/aarch64/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu")
#set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-gcc")
#set(CMAKE_CXX_COMPILER "${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-g++")
#set(CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_DIR}/aarch64-none-linux-gnu/libc")

set(TOOLCHAIN_DIR "${PROJECT_ROOT}/../Toolchain/aarch64-rk-linux_gcc-10.3.1")
set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-g++")
set(CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_DIR}/aarch64-none-linux-gnu/libc")
include_directories(${TOOLCHAIN_DIR}/aarch64-none-linux-gnu/libc/include)

# 搜索策略：只在目标环境查找库
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
