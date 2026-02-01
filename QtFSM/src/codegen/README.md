# Code Generation Module

The **Codegen** module is responsible for Forward Engineering: converting the visual FSM model into compliant C++ code that implements the State Pattern.

## Key Classes

### [CodeGenerator](CodeGenerator.h)
The main class that takes an `FSM` model as input and writes the corresponding `.h` and `.cpp` files to disk.

## Generated Code Structure

The generator produces code following the **State Pattern**:

- **Context Class**: The main class (named after the FSM) that maintains a pointer to structure the `CurrentState`.
- **State Interface**: An abstract base class defining the `enter()`, `exit()`, and event-handling methods.
- **Concrete States**: Classes for each state in the FSM, implementing the interface.

### Example Output

```cpp
class LightSwitch; // Forward decl

class LightSwitchState {
public:
    virtual void enter(LightSwitch* fsm) {}
    virtual void exit(LightSwitch* fsm) {}
    virtual void toggle(LightSwitch* fsm) {}
};

class OnState : public LightSwitchState {
    void toggle(LightSwitch* fsm) override;
};
```
