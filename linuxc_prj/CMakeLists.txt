cmake_minimum_required(VERSION 3.10)
project(linuxc_framework)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 关键修改：添加线程支持（解决之前检测失败的问题）
find_package(Threads REQUIRED)  # 在 project() 之后立即添加

# 添加子目录
add_subdirectory(src)

# 可执行文件
add_executable(main_app src/app/main.c)

# 显式链接线程库
target_link_libraries(main_app PRIVATE framework Threads::Threads)

# 添加编译选项：确保使用 -pthread 标志
target_compile_options(main_app PRIVATE -pthread)

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

# 第三方库集成示例
# add_subdirectory(third_party/some_lib)
# target_link_libraries(main_app PRIVATE some_lib)

# 安装规则（可选）
install(TARGETS main_app
    RUNTIME DESTINATION bin
    COMPONENT applications
)
