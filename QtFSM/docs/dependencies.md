# Dependencies Documentation

This document lists all dependencies required for building and running QtFSM.

## Required Dependencies

### Qt Framework

- **Version**: Qt 6.5.0 or later
- **Components**:
  - Qt6Core
  - Qt6Gui
  - Qt6Widgets
- **Installation**:
  - Linux: `sudo apt install qt6-base-dev qt6-tools-dev`
  - Windows: Download from [qt.io](https://www.qt.io/download)
  - macOS: `brew install qt@6`

### Build System

- **CMake**: 3.16 or later
  - Linux: `sudo apt install cmake`
  - Windows: Download from [cmake.org](https://cmake.org/download/)
  - macOS: `brew install cmake`

### Compiler

One of the following C++17 compatible compilers:

- **GCC**: 9.0 or later
- **Clang**: 10.0 or later
- **MSVC**: Visual Studio 2019 or later

### Version Control

- **Git**: Any recent version
  - Linux: `sudo apt install git`
  - Windows: Download from [git-scm.com](https://git-scm.com/)
  - macOS: `brew install git`

## Optional Dependencies

### Development Tools

- **Ninja**: Fast build system (recommended)
  - Linux: `sudo apt install ninja-build`
  - Windows: `choco install ninja`
  - macOS: `brew install ninja`

- **ccache**: Compiler cache for faster rebuilds
  - Linux: `sudo apt install ccache`
  - macOS: `brew install ccache`

### Code Quality Tools

- **clang-tidy**: Static analysis
  - Linux: `sudo apt install clang-tidy`
  
- **clang-format**: Code formatting
  - Linux: `sudo apt install clang-format`

- **cppcheck**: Additional static analysis
  - Linux: `sudo apt install cppcheck`

### Debugging Tools

- **GDB**: GNU Debugger
  - Linux: `sudo apt install gdb`
  
- **Valgrind**: Memory leak detection
  - Linux: `sudo apt install valgrind`

## Testing Dependencies

- **Qt Test**: Included with Qt6
- **CTest**: Included with CMake

## CI/CD Dependencies

For the GitLab CI pipeline:

- Docker
- Ubuntu 22.04 base image
- All required dependencies listed above

## Dependency Version Matrix

| Component | Minimum | Recommended | Tested With |
|-----------|---------|-------------|-------------|
| Qt        | 6.5.0   | 6.5.3       | 6.5.3       |
| CMake     | 3.16.0  | 3.27.0      | 3.27.7      |
| GCC       | 9.0     | 11.0        | 11.4.0      |
| Clang     | 10.0    | 15.0        | 15.0.0      |
| MSVC      | 2019    | 2022        | 2022        |

## Platform-Specific Notes

### Linux (Ubuntu 22.04)

All dependencies available through apt:
```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    qt6-tools-dev \
    libgl1-mesa-dev
```

### Windows

- Install Visual Studio 2019+ with C++ workload
- Install Qt using Qt Online Installer
- Install CMake and Git (or use Qt's bundled versions)

### WSL2

Same as Linux, works perfectly with X11 forwarding:
```bash
# Enable X11 in WSL
export DISPLAY=:0
```

### macOS

Use Homebrew for easy installation:
```bash
brew install qt@6 cmake git
```

## Troubleshooting

### Qt not found by CMake

Set `CMAKE_PREFIX_PATH` to Qt installation:
```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.5.3/gcc_64 ..
```

### Missing OpenGL libraries (Linux)

```bash
sudo apt install libgl1-mesa-dev libglu1-mesa-dev
```

### Qt Creator can't find kit

1. Go to Tools > Options > Kits
2. Check that Qt version and compiler are detected
3. Create a new kit if needed

## Updating Dependencies

To update Qt:
1. Use Qt Maintenance Tool
2. Select new version
3. Update `CMakeLists.txt` if API changes

To update CMake presets:
1. Edit `CMakePresets.json`
2. Update `cmakeMinimumRequired` if needed
