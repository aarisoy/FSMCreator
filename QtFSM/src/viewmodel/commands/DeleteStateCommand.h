#ifndef DELETESTATECOMMAND_H
#define DELETESTATECOMMAND_H

#include <QList>
#include <QPointF>
#include <QString>
#include <QUndoCommand>


class FSM;
class State;
class Transition;

/**
 * @brief Command to delete a state with undo/redo support
 */
class DeleteStateCommand : public QUndoCommand {
public:
  DeleteStateCommand(FSM *fsm, State *state, QUndoCommand *parent = nullptr);
  ~DeleteStateCommand();

  void undo() override;
  void redo() override;

private:
  FSM *m_fsm;
  State *m_state;
  QString m_stateId;
  QString m_stateName;
  QPointF m_position;
  bool m_wasInitial;

  // Store connected transitions to restore them on undo
  struct TransitionData {
    State *source;
    State *target;
    QString event;
  };
  QList<TransitionData> m_connectedTransitions;
};

#endif // DELETESTATECOMMAND_H
