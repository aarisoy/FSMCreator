#!/bin/bash
# WSL2 Quick Setup for QtFSM
# Run this script to install everything you need in one go

set -e

echo "=========================================="
echo "QtFSM WSL2 Environment Setup"
echo "=========================================="
echo ""

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_status() {
    echo -e "${BLUE}==>${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}!${NC} $1"
}

# Step 1: Update system
print_status "Updating package lists..."
sudo apt update

# Step 2: Install build tools
print_status "Installing build tools (gcc, make, cmake)..."
sudo apt install -y build-essential cmake git ninja-build

print_success "Build tools installed"

# Step 3: Install Qt6
print_status "Installing Qt6 (this may take a few minutes)..."
sudo apt install -y \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6

print_success "Qt6 installed"

# Step 4: Install OpenGL (required for Qt GUI)
print_status "Installing OpenGL libraries..."
sudo apt install -y libgl1-mesa-dev libglu1-mesa-dev

print_success "OpenGL libraries installed"

# Step 5: Optional development tools
print_status "Installing optional development tools..."
sudo apt install -y clang-format clang-tidy gdb valgrind

print_success "Development tools installed"

# Step 6: Set up X11 display
print_status "Configuring X11 display..."
if ! grep -q "DISPLAY=:0" ~/.bashrc; then
    echo 'export DISPLAY=:0' >> ~/.bashrc
    print_success "Added DISPLAY=:0 to ~/.bashrc"
else
    print_warning "DISPLAY already configured in ~/.bashrc"
fi

export DISPLAY=:0

echo ""
echo "=========================================="
print_success "Installation Complete!"
echo "=========================================="
echo ""

# Verify installations
echo "Verifying installations..."
echo ""
cmake --version | head -n 1
echo ""
qmake6 --version 2>/dev/null || qmake --version | head -n 1
echo ""
g++ --version | head -n 1
echo ""

echo "=========================================="
echo "Next Steps:"
echo "=========================================="
echo ""
echo "1. Install X Server on Windows:"
echo "   - Download VcXsrv: https://sourceforge.net/projects/vcxsrv/"
echo "   - Launch XLaunch, select 'Multiple windows', Display 0"
echo ""
echo "2. Reload your shell:"
echo "   source ~/.bashrc"
echo ""
echo "3. Build QtFSM:"
echo "   cd /mnt/c/Users/Pc/Desktop/QtFSM"
echo "   cmake --preset dev"
echo "   cmake --build build"
echo ""
echo "4. Run the application:"
echo "   ./build/QtFSM"
echo ""
print_success "Ready to build QtFSM!"
