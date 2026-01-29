# QtFSM - Finite State Machine Designer & Code Generator

A bidirectional FSM tool that generates C++ State pattern code from visual diagrams and creates diagrams from existing C++ code.

## Features

- 🎨 **Visual FSM Editor**: Draw state machines with an intuitive drag-and-drop interface
- 💻 **Code Generation**: Generate clean C++ State pattern code from diagrams
- 🔍 **Code Parsing**: Import and visualize existing C++ FSM implementations
- 💾 **Project Management**: Save/load FSM projects in JSON format
- ↩️ **Undo/Redo**: Full undo/redo support for all operations
- 🎯 **MVVM Architecture**: Clean separation of concerns for maintainability

## Requirements

### Development

- **Qt 6.5+** (Core, Gui, Widgets)
- **CMake 3.16+**
- **C++17 compatible compiler** (gcc 9+, clang 10+, MSVC 2019+)
- **Git**

### Build Tools

- CMake
- Ninja (optional, but recommended)
- ccache (optional, for faster rebuilds)

## Quick Start

### Linux/WSL2

```bash
# 1. Install dependencies
chmod +x scripts/setup_dev_env.sh
./scripts/setup_dev_env.sh

# 2. Build the project
cmake --preset dev
cmake --build build

# 3. Run
./build/QtFSM
```

### Windows

```powershell
# 1. Check environment (and install missing tools)
.\scripts\setup_dev_env.ps1

# 2. Build the project
cmake --preset dev
cmake --build build --config Debug

# 3. Run
.\build\Debug\QtFSM.exe
```

## Development Setup

### Using Qt Creator (Recommended)

1. Open Qt Creator
2. File > Open File or Project
3. Select `CMakeLists.txt`
4. Choose your kit (Desktop Qt 6.x)
5. Configure and Build

### Using VSCode

1. Install CMake Tools extension
2. Open project folder
3. Select CMake preset (dev or release)
4. Build and run

## Project Structure

```
QtFSM/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── model/                # FSM data model
│   ├── viewmodel/            # MVVM logic layer
│   ├── view/                 # Qt Widgets UI
│   ├── codegen/              # Code generation
│   ├── parser/               # Code parsing
│   └── serialization/        # JSON save/load
├── tests/                    # Unit and integration tests
├── examples/                 # Sample FSM projects
├── scripts/                  # Setup and utility scripts
├── docker/                   # Docker configurations
├── CMakeLists.txt            # Build configuration
├── CMakePresets.json         # Build presets
└── .gitlab-ci.yml            # CI/CD pipeline
```

## Building

### CMake Presets

```bash
# Development (Debug with tests)
cmake --preset dev
cmake --build build

# Release (Optimized)
cmake --preset release
cmake --build build-release
```

### Manual CMake

```bash
# Debug build
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build

# Release build
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

## Testing

```bash
# Build with tests enabled
cmake --preset dev
cmake --build build

# Run tests
cd build
ctest --output-on-failure
```

## Usage

### Creating an FSM

1. Launch QtFSM
2. Add states using the toolbar or right-click menu
3. Connect states with transitions
4. Edit state/transition properties in the properties panel
5. Generate C++ code: Tools > Generate Code

### Importing from Code

1. File > Import from Code
2. Select C++ files containing FSM implementation
3. FSM will be visualized automatically
4. Edit and regenerate as needed

## Architecture

QtFSM follows the **MVVM (Model-View-ViewModel)** pattern:

- **Model**: Core FSM data structures (State, Transition, Event)
- **View**: Qt Widgets UI (QGraphicsView, custom items)
- **ViewModel**: Mediates between Model and View, handles commands

For detailed architecture information, see the [Design Document](docs/design.md).

## CI/CD

The project uses GitLab CI/CD with the following pipeline:

- **Build**: Multi-platform builds (Linux, Windows, macOS)
- **Test**: Automated unit and integration tests
- **Quality**: Static analysis, linting, code coverage
- **Package**: Create distributable packages
- **Deploy**: Release artifacts on tags

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Merge Request

## License

[TODO: Add license information]

## Roadmap

- [x] MVVM architecture design
- [x] Project structure setup
- [ ] Core model implementation
- [ ] Visual diagram editor
- [ ] Code generation (State pattern)
- [ ] Code parsing
- [ ] Undo/Redo system
- [ ] Advanced features (zoom, pan, export diagrams)

## Support

For issues, questions, or suggestions, please open an issue on GitLab.

---

**Status**: 🚧 Under Active Development
