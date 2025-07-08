cmake_minimum_required(VERSION 3.22.1)

project("native-lib")

# Recursively find all source files
file(GLOB_RECURSE PROJECT_SOURCES
        "${CMAKE_SOURCE_DIR}/*.c"
        "${CMAKE_SOURCE_DIR}/*.cpp"
)

# Default Android build
add_library(${CMAKE_PROJECT_NAME} SHARED ${PROJECT_SOURCES})

# Link Android libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        log
)