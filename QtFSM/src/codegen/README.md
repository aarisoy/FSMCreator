# Code Generation Module

The **Codegen** module is responsible for Forward Engineering: converting the visual FSM model into a **config-driven C++ runtime**.

## Key Classes

### [CodeGenerator](CodeGenerator.h)
The main class that takes an `FSM` model as input and writes a single C++ translation unit containing:

- Config structs (`FSMConfig`, `StateConfig`, `TransitionConfig`)
- A minimal runtime interpreter (`ConfigFSM`)
- A config instance (`cfg`) populated from the visual model

## Generated Code Structure

The generator produces code following a **config-based** approach:

- **Config Data**: All states, transitions, and UI positions are emitted as data.
- **Runtime Interpreter**: A small engine that evaluates events and transitions at runtime.
- **Action/Guard Registry**: Optional mapping from string IDs to functions.

### Example Output (excerpt)

```cpp
struct TransitionConfig {
    std::string event;
    std::string to;
    std::string guard;
    std::string action;
};

struct StateConfig {
    std::string name;
    double x;
    double y;
    bool isFinal;
    std::string entry;
    std::string exit;
    std::vector<TransitionConfig> transitions;
};

struct FSMConfig {
    std::string initial;
    std::unordered_map<std::string, StateConfig> states;
};
```
