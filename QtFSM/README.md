# QtFSM - Finite State Machine Designer & Code Generator

![Status](https://img.shields.io/badge/Status-Active_Development-blue)
![License](https://img.shields.io/badge/License-MIT-green)
![Qt](https://img.shields.io/badge/Qt-6.5+-success)
![C++](https://img.shields.io/badge/C++-17-blue)

A professional-grade, bidirectional Finite State Machine (FSM) tool designed to bridge the gap between visual design and robust C++ implementation. QtFSM allows developers to generate production-ready C++ State pattern code from visual diagrams and, uniquely, reverse-engineer diagrams from existing C++ FSM code.

## 🌟 Key Features

- **🎨 Visual FSM Editor**: deeply integrated, drag-and-drop canvas for designing complex state machines.
- **🔄 Bidirectional Engineering**:
    - **Forward**: Generate compliant C++ State Pattern code.
    - **Reverse**: Import existing C++ headers/sources to visualize FSM logic.
- **🏗️ Solid Architecture**: Built on a strict **MVVM (Model-View-ViewModel)** architecture ensuring separation of concerns.
- **↩️ Robust Undo/Redo**: Command-based history system for all edit operations.
- **💾 Persistence**: JSON-based project format for saving and sharing designs.
- **🔌 Extensible Modules**: Modular design with separate subsystems for parsing, generation, serialization, and UI.

---

## 📐 Design & Architecture

QtFSM is built with maintainability and scalability in front of mind, adhering to **SOLID principles** and modern C++ best practices.

### Architectural Patter: MVVM (Model-View-ViewModel)

We chose MVVM to decouple the UI (View) from the business logic (Model).

- **Model (`src/model`)**: Pure C++ data structures representing the FSM (States, Transitions, Events). It has no dependency on Qt's GUI classes, making it portable and testable.
- **ViewModel (`src/viewmodel`)**: The glue layer. It wraps the Model and exposes data to the View via Qt's Signals & Slots. It handles the "business logic" of the UI interactions.
- **View (`src/view`)**: The presentation layer. Built with Qt Graphics View Framework (`QGraphicsScene`, `QGraphicsView`). It observes the ViewModel and renders the state machine.

### Design Patterns Used

- **Command Pattern**: Used for the Undo/Redo system. Every action (Move State, Add Transition, etc.) is encapsulated as a `QUndoCommand`.
- **State Pattern**: The code generator creates code following the standard GoF State Pattern.
- **Observer Pattern**: Heavily used via Qt's Signals/Slots for data binding between ViewModel and View.
- **Factory Pattern**: Used in formatting and parsing logic.

---

## 📂 Project Structure

The codebase is organized into distinct modules:

| Module | Description |
|--------|-------------|
| **`src/model`** | Core data entities (`State`, `Transition`, `FSM`). Independent of UI. |
| **`src/view`** | GUI components, Dialogs, and Graphics Items (`StateItem`, `TransitionItem`). |
| **`src/viewmodel`** | Logic controllers (`MainViewModel`, `DiagramViewModel`) managing application state. Includes **Commands**. |
| **`src/parsing`** | Clang/Regex-based C++ parsers to reconstruct FSMs from code. |
| **`src/codegen`** | Template-based C++ code generators. |
| **`src/serialization`** | JSON serializers/deserializers for project persistence. |

---

## 🚀 Getting Started

### Prerequisites

- **Qt 6.5+** (Core, Gui, Widgets)
- **CMake 3.16+**
- **C++17 Compiler** (GCC 9+, Clang 10+, MSVC 2019+)

### Build Instructions

#### Linux / macOS
```bash
# 1. Setup (Optional)
./scripts/setup_dev_env.sh

# 2. Build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# 3. Run
./build/QtFSM
```

#### Windows (PowerShell)
```powershell
# 1. Build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# 2. Run
.\build\Debug\QtFSM.exe
```

---

## 🛠️ Usage Workflow

1. **Design**: Launch the app and use the toolbar to add States. Drag between states to create Transitions.
2. **Configure**: Click on a State or Transition to edit its properties (Name, Entry/Exit Actions, Guard Conditions) in the side panel.
3. **Generate**: Go to **Tools > Generate Code** to export your FSM as C++ classes.
4. **Iterate**: Save your project (`.fsm.json`) and return later to make changes.

---

## 📄 Documentation

Detailed documentation for each module can be found in their respective directories:

- [Model Documentation](src/model/README.md)
- [View Documentation](src/view/README.md)
- [ViewModel Documentation](src/viewmodel/README.md)
- [Parsing Documentation](src/parsing/README.md)
- [Code Generation Documentation](src/codegen/README.md)
- [Serialization Documentation](src/serialization/README.md)

---

## 🤝 Contributing

Contributions are welcome! Please follow the `CONTRIBUTING.md` guidelines.
1. Fork the repo.
2. Create a feature branch.
3. Submit a Pull Request.

---

**© 2026 QtFSM Team**

