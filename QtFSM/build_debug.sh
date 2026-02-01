#!/bin/bash
set -e

echo "=== Building QtFSM (Debug) ==="

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure CMake in Debug mode
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the main application
cmake --build . --target QtFSM -j$(nproc)

echo "-----------------------------------"
echo "Debug Build SUCCESSFUL!"
echo "Binary: build/QtFSM"
echo "-----------------------------------"
