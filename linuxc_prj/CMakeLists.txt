# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(linuxc_framework)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 添加子目录
add_subdirectory(src)

# 可执行文件
add_executable(main_app src/app/main.c)
target_link_libraries(main_app framework)

# 测试
if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()

# 第三方库集成示例
# add_subdirectory(third_party/some_lib)
# target_link_libraries(main_app some_lib)
