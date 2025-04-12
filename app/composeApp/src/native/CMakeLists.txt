cmake_minimum_required(VERSION 3.22.1)
project("pocketPixel")

# Set the path to your SDL2 and SDL2_ttf directories
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/SDL2/SDL")
set(SDL2_TTF_DIR "${CMAKE_SOURCE_DIR}/SDL2_ttf")

# Add SDL2 and SDL2_ttf subdirectories
add_subdirectory(${SDL2_DIR} ${CMAKE_BINARY_DIR}/SDL2_build)
add_subdirectory(${SDL2_TTF_DIR} ${CMAKE_BINARY_DIR}/SDL2_ttf_build)

# Recursively find all source files
file(GLOB_RECURSE PROJECT_SOURCES
        "${CMAKE_SOURCE_DIR}/*.c"
        "${CMAKE_SOURCE_DIR}/*.cpp"
        "${CMAKE_SOURCE_DIR}/GameBoyCore.cpp"
)

# Create the shared library
add_library(${CMAKE_PROJECT_NAME} SHARED ${PROJECT_SOURCES})

# Include SDL2 and SDL2_ttf headers
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${SDL2_DIR}/include
        ${SDL2_TTF_DIR}/include
)

# Link SDL2, SDL2_ttf, and Android libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
        SDL2
        SDL2_ttf
        android
        log
)
