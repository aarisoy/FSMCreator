#ifndef DELETETRANSITIONCOMMAND_H
#define DELETETRANSITIONCOMMAND_H

#include <QString>
#include <QUndoCommand>


class FSM;
class State;
class Transition;

class DeleteTransitionCommand : public QUndoCommand {
public:
  DeleteTransitionCommand(FSM *fsm, Transition *transition,
                          QUndoCommand *parent = nullptr);
  ~DeleteTransitionCommand();

  void undo() override;
  void redo() override;

private:
  FSM *m_fsm;
  Transition *m_transition;
  State *m_source;
  State *m_target;
  QString m_event;
};

#endif // DELETETRANSITIONCOMMAND_H
