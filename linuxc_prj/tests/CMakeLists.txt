# tests/CMakeLists.txt
enable_testing()

# 添加日志测试
add_executable(test_logger test_logger.c)
target_link_libraries(test_logger framework)
add_test(NAME test_logger COMMAND test_logger)

# 添加GPIO测试
add_executable(test_gpio test_gpio.c)
target_link_libraries(test_gpio framework)
add_test(NAME test_gpio COMMAND test_gpio)
