# Serialization Module

The **Serialization** module manages the saving and loading of FSM models to permanent storage, using JSON as the data format.

## Key Classes

### [JSONSerializer](JSONSerializer.h)
Handles the conversion between the `FSM` object graph and `QJsonObject`/`QJsonDocument`.

- **Saving**: Traverses the FSM's states and transitions, writing their properties (ID, name, position, events, guards, actions) to a JSON structure.
- **Loading**: Reads a JSON file, validates the structure, and reconstructs the FSM.

## JSON Format Example

```json
{
    "states": [
        {
            "id": "s1",
            "name": "Idle",
            "position": {"x": 100, "y": 100},
            "initial": true
        },
        {
            "id": "s2",
            "name": "Running",
            "position": {"x": 300, "y": 100}
        }
    ],
    "transitions": [
        {
            "from": "s1",
            "to": "s2",
            "event": "start"
        }
    ]
}
```
