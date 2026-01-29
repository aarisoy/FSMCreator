# Development Environment Setup Script for Windows
# This script guides you through setting up the QtFSM development environment

Write-Host "=========================================="
Write-Host "QtFSM Development Environment Setup"
Write-Host "=========================================="
Write-Host ""

function Print-Status {
    param([string]$message)
    Write-Host "==> " -ForegroundColor Blue -NoNewline
    Write-Host $message
}

function Print-Success {
    param([string]$message)
    Write-Host "✓ " -ForegroundColor Green -NoNewline
    Write-Host $message
}

function Print-Warning {
    param([string]$message)
    Write-Host "⚠ " -ForegroundColor Yellow -NoNewline
    Write-Host $message
}

# Check for CMake
Print-Status "Checking for CMake..."
try {
    $cmakeVersion = cmake --version
    Print-Success "CMake is installed: $($cmakeVersion[0])"
} catch {
    Print-Warning "CMake not found. Please install CMake from https://cmake.org/download/"
    Write-Host "   Or use: winget install Kitware.CMake"
}

Write-Host ""

# Check for Qt
Print-Status "Checking for Qt..."
try {
    $qmakeVersion = qmake --version
    Print-Success "Qt is installed"
} catch {
    Print-Warning "Qt not found. Please install Qt from https://www.qt.io/download-qt-installer"
    Write-Host "   Make sure to install Qt 6.5 or later with the following components:"
    Write-Host "   - Qt 6.x for MSVC 2019 64-bit (or MinGW)"
    Write-Host "   - Qt Creator"
    Write-Host "   - CMake"
}

Write-Host ""

# Check for Git
Print-Status "Checking for Git..."
try {
    $gitVersion = git --version
    Print-Success "Git is installed: $gitVersion"
} catch {
    Print-Warning "Git not found. Please install Git from https://git-scm.com/download/win"
    Write-Host "   Or use: winget install Git.Git"
}

Write-Host ""
Write-Host "=========================================="
Write-Host "Setup Instructions"
Write-Host "=========================================="
Write-Host ""
Write-Host "If any tools are missing, please install them and run this script again."
Write-Host ""
Write-Host "To build the project:"
Write-Host "  1. Open Qt Creator"
Write-Host "  2. File > Open File or Project > Select CMakeLists.txt"
Write-Host "  3. Configure project with your preferred kit"
Write-Host "  4. Build and run"
Write-Host ""
Write-Host "Or from command line:"
Write-Host "  1. cmake --preset dev"
Write-Host "  2. cmake --build build"
Write-Host ""
