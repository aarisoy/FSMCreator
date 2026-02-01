#!/bin/bash
set -e

echo "=== Building and Running Tests ==="

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure CMake in Debug mode
cmake .. -DCMAKE_BUILD_TYPE=Debug

# List of test targets
TESTS=(
    "test_stress"
    "test_user_code"
    "test_json"
    "test_lexer"
    "test_parser"
    "test_debug_parser"
)

# Build and run each test
for test_target in "${TESTS[@]}"; do
    echo "-----------------------------------"
    echo "Building $test_target..."
    cmake --build . --target $test_target -j$(nproc)
    
    echo "Running $test_target..."
    if ./$test_target; then
        echo "[PASS] $test_target"
    else
        echo "[FAIL] $test_target"
        exit 1
    fi
done

echo "-----------------------------------"
echo "All Tests PASSED!"
echo "-----------------------------------"
