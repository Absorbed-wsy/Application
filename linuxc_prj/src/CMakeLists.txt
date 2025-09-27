# src/CMakeLists.txt
# 包含头文件目录
include_directories(../include)

include_directories(algo app core db modules net peripherals util)

# 添加源文件
file(GLOB_RECURSE SOURCES "*.c")

# 创建库
add_library(framework STATIC ${SOURCES})

# pthread链接库
find_package(Threads REQUIRED)
target_link_libraries(framework PRIVATE Threads::Threads)

# find_package(SQLite3 REQUIRED)
# target_link_libraries(framework PRIVATE SQLite::SQLite3)
