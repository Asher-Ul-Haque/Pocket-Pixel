cmake_minimum_required(VERSION 3.22.1)

project("native-lib")

# Recursively find all source files
file(GLOB_RECURSE PROJECT_SOURCES
        "${CMAKE_SOURCE_DIR}/src/*.c"
        "${CMAKE_SOURCE_DIR}/*.cpp"
)

# Add native shared library
add_library(${CMAKE_PROJECT_NAME} SHARED ${PROJECT_SOURCES})

# Add include directory
target_include_directories(${CMAKE_PROJECT_NAME}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Link necessary Android libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        GLESv2
        log
)

# Apply aggressive optimization and size reduction flags
set(CMAKE_C_FLAGS_RELEASE
        "${CMAKE_C_FLAGS_RELEASE} -O3 -flto -fdata-sections -ffunction-sections"
)
set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto -fdata-sections -ffunction-sections"
)
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE
        "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--gc-sections"
)