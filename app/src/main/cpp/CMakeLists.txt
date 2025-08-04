cmake_minimum_required(VERSION 3.22.1)

project("native-lib")

find_package (REQUIRED CONFIG)

# Recursively find all source files
file(GLOB_RECURSE PROJECT_SOURCES
        "${CMAKE_SOURCE_DIR}/*.c"
        "${CMAKE_SOURCE_DIR}/*.cpp"
        "${CMAKE_SOURCE_DIR}/GameBoyCore.cpp"
        "${CMAKE_SOURCE_DIR}/NativeBridge.cpp"
)

# Add native shared library
add_library(${CMAKE_PROJECT_NAME} SHARED ${PROJECT_SOURCES})

# Link necessary Android libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        GLESv2
        log
)

# Define compile-time flags to disable logs
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        LOG_WARNING_ENABLED=0
        LOG_INFO_ENABLED=0
        LOG_DEBUG_ENABLED=0
        LOG_TRACE_ENABLED=0
)

# Apply aggressive optimization and size reduction flags
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -flto -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto -fdata-sections -ffunction-sections")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")