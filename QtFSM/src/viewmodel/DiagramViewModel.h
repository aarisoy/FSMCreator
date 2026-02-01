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
class DiagramViewModel : public QObject {
  Q_OBJECT

public:
  explicit DiagramViewModel(QObject *parent = nullptr);
  ~DiagramViewModel();

  // FSM access
  FSM *fsm() const;
  void setFSM(FSM *fsm);

  // Command execution methods
  void addState(const QString &id, const QString &name,
                const QPointF &position);
  void deleteState(State *state);
  void addTransition(State *source, State *target, const QString &event);
  void deleteTransition(Transition *transition);

  // Undo/Redo
  void undo();
  void redo();
  bool canUndo() const;
  bool canRedo() const;
  QString undoText() const;
  QString redoText() const;

  QUndoStack *undoStack() const;

signals:
  // Notify views of changes
  void stateAdded(State *state);
  void stateRemoved(State *state);
  void transitionAdded(Transition *transition);
  void transitionRemoved(Transition *transition);
  void undoRedoStateChanged();
  void modelChanged(); // Emitted whenever the model changes (add, delete, undo,
                       // redo)

private:
  FSM *m_fsm;
  QUndoStack *m_undoStack;
};

#endif // DIAGRAMVIEWMODEL_H
