cmake_minimum_required(VERSION 3.10)
project(linuxc_framework)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(src)

# Add header file include paths
include_directories(src src/algo src/app src/core src/db src/modules src/net src/peripherals src/util)

# Executable file
add_executable(main_app src/app/main.c)

# Add library support
target_link_libraries(main_app PRIVATE framework)


# Cross-compilation compatibility: Set target properties
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for ${CMAKE_SYSTEM_PROCESSOR}")
    # Set target system properties
    set_target_properties(main_app PROPERTIES
        LINK_FLAGS "-Wl,--no-undefined"
    )
else()
    message(STATUS "Native compiling for ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Tests
option(ENABLE_TESTS "Enable tests" OFF)
if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()

# Installation rules (optional)
install(TARGETS main_app
    RUNTIME DESTINATION bin
    COMPONENT applications
)
