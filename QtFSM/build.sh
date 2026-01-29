#!/bin/bash

# QtFSM Build Script for Linux/WSL

echo "Building QtFSM..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

# Enter build directory
cd build

# Configure with CMake
cmake ..

# Check if CMake failed
if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Build with Make (using all available cores)
make -j$(nproc)

# Check if Make failed
if [ $? -ne 0 ]; then
    echo "Build FAILED!"
    exit 1
else
    echo "-----------------------------------"
    echo "Build SUCCESSFUL!"
    echo "Binary is at: build/QtFSM"
    echo "-----------------------------------"
fi
