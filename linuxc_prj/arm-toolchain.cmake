# arm-toolchain.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)

# Specify cross-compiler path / Set library search path (adjust according to actual path)
# RK3399
set(TOOLCHAIN_DIR "${PROJECT_ROOT}/../Toolchain/aarch64-rk-linux_gcc-10.3.1")
set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-g++")
set(CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_DIR}/aarch64-none-linux-gnu/libc")
include_directories(${TOOLCHAIN_DIR}/aarch64-none-linux-gnu/libc/include)

# Search strategy: Only search libraries in target environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
