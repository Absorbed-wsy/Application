cmake_minimum_required(VERSION 3.10)
project(linuxc_framework)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 添加子目录
add_subdirectory(src)

# 添加头文件包含路径
include_directories(src src/algo src/app src/core src/db src/modules src/net src/peripherals src/util)

# 可执行文件
add_executable(main_app src/app/main.c)

# 添加库支持
target_link_libraries(main_app PRIVATE framework)


# 交叉编译兼容性：设置目标属性
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for ${CMAKE_SYSTEM_PROCESSOR}")
    # 设置目标系统属性
    set_target_properties(main_app PROPERTIES
        LINK_FLAGS "-Wl,--no-undefined"
    )
else()
    message(STATUS "Native compiling for ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# 测试
option(ENABLE_TESTS "Enable tests" OFF)
if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()

# 安装规则（可选）
install(TARGETS main_app
    RUNTIME DESTINATION bin
    COMPONENT applications
)
