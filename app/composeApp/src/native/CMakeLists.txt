cmake_minimum_required(VERSION 3.22.1)

project("pocketPixel")


# Recursively find all source files
file(GLOB_RECURSE PROJECT_SOURCES
        "${CMAKE_SOURCE_DIR}/*.c"
        "${CMAKE_SOURCE_DIR}/*.cpp"
        "${CMAKE_SOURCE_DIR}/GameBoyCore.cpp"
)


# Default Android build
add_library(${CMAKE_PROJECT_NAME} SHARED ${PROJECT_SOURCES})

# Platform-specific settings for Android
target_link_libraries(${CMAKE_PROJECT_NAME} android log)