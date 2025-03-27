#!/bin/bash

# Check if EMSDK is already sourced
source /home/nadeem/Downloads/emsdk/emsdk_env.sh

# Define directories
BUILD_DIR=src/wasmJsMain/resources
SRC_DIR=src/native
SCRIPT_DIR=$(dirname "$0")

# Ensure the script is run from the project root
cd "$SCRIPT_DIR" || { echo "Failed to change directory"; exit 1; }

# Clean and recreate build directories
mkdir -p $BUILD_DIR $OUTPUT_DIR

# Compile using emcc with the original command
emcc $SRC_DIR/WebBridge.cpp -o $BUILD_DIR/pocketPixel.js \
    -s EXPORTED_FUNCTIONS="['_getFrame']" \
    -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" || { echo "Build failed"; exit 1; } \


echo "WebAssembly build completed. Files are in $BUILD_DIR/"
