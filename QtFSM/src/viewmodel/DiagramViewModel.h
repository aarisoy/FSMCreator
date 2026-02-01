#ifndef DIAGRAMVIEWMODEL_H
#define DIAGRAMVIEWMODEL_H

#include <QObject>
#include <QUndoStack>

class FSM;
class State;
class Transition;

/**
 * @brief The DiagramViewModel class - ViewModel for diagram editor
 * Manages FSM operations with undo/redo support via Command pattern
 */
/**
 * @brief The DiagramViewModel class mediates between the FSM Model and the
 * Diagram View.
 *
 * It manages the selection state, handles undo/redo stacks, and exposes methods
 * for manipulating the FSM structure (adding/removing states/transitions) which
 * are then executed via Command objects.
 *
 * @ingroup ViewModel
 */
class DiagramViewModel : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Constructs a new DiagramViewModel.
   * @param parent The parent QObject (default: nullptr).
   */
  explicit DiagramViewModel(QObject *parent = nullptr);

  /**
   * @brief Destroys the DiagramViewModel.
   */
  ~DiagramViewModel();

  // FSM access
  /**
   * @brief Gets the associated FSM model.
   * @return Pointer to the FSM.
   */
  FSM *fsm() const;

  /**
   * @brief Sets the FSM model to view/edit.
   * @param fsm Pointer to the FSM.
   */
  void setFSM(FSM *fsm);

  // Command execution methods
  /**
   * @brief Adds a new state to the FSM.
   * Creates an undo-able command.
   * @param id The unique identifier for the state.
   * @param name The display name of the state.
   * @param position The 2D position on the diagram.
   */
  void addState(const QString &id, const QString &name,
                const QPointF &position);

  /**
   * @brief Deletes a state from the FSM.
   * Creates an undo-able command.
   * @param state The state to delete.
   */
  void deleteState(State *state);

  /**
   * @brief Adds a new transition between two states.
   * Creates an undo-able command.
   * @param source The source state.
   * @param target The target state.
   * @param event The triggering event name.
   */
  void addTransition(State *source, State *target, const QString &event);

  /**
   * @brief Deletes a transition from the FSM.
   * Creates an undo-able command.
   * @param transition The transition to delete.
   */
  void deleteTransition(Transition *transition);

  // Undo/Redo
  /**
   * @brief Undoes the last command.
   */
  void undo();

  /**
   * @brief Redoes the last undone command.
   */
  void redo();

  /**
   * @brief Checks if undo is possible.
   * @return true if there are commands to undo.
   */
  bool canUndo() const;

  /**
   * @brief Checks if redo is possible.
   * @return true if there are commands to redo.
   */
  bool canRedo() const;

  /**
   * @brief Gets the text description of the undo action.
   */
  QString undoText() const;

  /**
   * @brief Gets the text description of the redo action.
   */
  QString redoText() const;

  /**
   * @brief Gets the internal undo stack.
   * @return Pointer to the QUndoStack.
   */
  QUndoStack *undoStack() const;

signals:
  // Notify views of changes
  /**
   * @brief Emitted when a state is added to the model.
   */
  void stateAdded(State *state);

  /**
   * @brief Emitted when a state is removed from the model.
   */
  void stateRemoved(State *state);

  /**
   * @brief Emitted when a transition is added to the model.
   */
  void transitionAdded(Transition *transition);

  /**
   * @brief Emitted when a transition is removed from the model.
   */
  void transitionRemoved(Transition *transition);

  /**
   * @brief Emitted when the undo/redo stack state changes (e.g., canUndo
   * changes).
   */
  void undoRedoStateChanged();

  /**
   * @brief Emitted whenever the model changes (add, delete, undo, redo).
   */
  void modelChanged(); // Emitted whenever the model changes (add, delete, undo,
                       // redo)

private:
  FSM *m_fsm;
  QUndoStack *m_undoStack;
};

#endif // DIAGRAMVIEWMODEL_H
