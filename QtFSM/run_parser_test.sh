#!/bin/bash
set -e

# Ensure we are in the script's directory (QtFSM root)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "Building test_debug_parser using CMake..."

# Configure if build dir doesn't exist or Makefile is missing
if [ ! -d "build" ] || [ ! -f "build/Makefile" ]; then
    cmake -B build -DCMAKE_BUILD_TYPE=Debug
fi

# Build the target
cmake --build build --target test_debug_parser

# Run the test
echo "Running test_debug_parser..."
./build/test_debug_parser
