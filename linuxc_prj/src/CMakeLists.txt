# src/CMakeLists.txt
# 包含头文件目录
include_directories(../include)

# 添加源文件
file(GLOB_RECURSE SOURCES "*.c")

# 创建库
add_library(framework STATIC ${SOURCES})

# 链接库
find_package(Threads REQUIRED)
target_link_libraries(framework PRIVATE Threads::Threads)

# 对于其他库，如SQLite3，按需添加
# find_package(SQLite3 REQUIRED)
# target_link_libraries(framework PRIVATE SQLite::SQLite3)
