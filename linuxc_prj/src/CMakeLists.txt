# src/CMakeLists.txt
# Include header directories
include_directories(../include)

include_directories(algo app core db modules net peripherals util)

# Add source files
file(GLOB_RECURSE SOURCES "*.c")

# Create library
add_library(framework STATIC ${SOURCES})

# pthread library linking
find_package(Threads REQUIRED)
target_link_libraries(framework PRIVATE Threads::Threads)

# libgpiod library linking
find_library(GPIOD_LIBRARY gpiod)
if(GPIOD_LIBRARY)
    target_link_libraries(framework PRIVATE ${GPIOD_LIBRARY})
    message(STATUS "Found libgpiod: ${GPIOD_LIBRARY}")
else()
    message(WARNING "libgpiod not found, GPIO functionality will be disabled")
endif()
