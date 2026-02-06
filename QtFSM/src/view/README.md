# View Module

The **View** module contains the Qt widgets that render and interact with the FSM UI.

## Key Classes

- **[DiagramEditor](DiagramEditor.h)**: The central canvas where the FSM is drawn. It hosts the `QGraphicsScene`.
- **[CodePreviewPanel](CodePreviewPanel.h)**: A dock widget showing the live-generated **C++ config**.
- **[TransitionItem](TransitionItem.h)**: Visual representation of a Transition (arrow between states). Calculates strict/curved paths and arrowheads.
- **[StateDialog](StateDialog.h)**: (Legacy/Alternative) Modal dialog to add/edit states.
- **[TransitionDialog](TransitionDialog.h)**: (Legacy/Alternative) Modal dialog to add/edit transitions.
