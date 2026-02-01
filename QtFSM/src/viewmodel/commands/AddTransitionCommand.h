#ifndef ADDTRANSITIONCOMMAND_H
#define ADDTRANSITIONCOMMAND_H

#include <QString>
#include <QUndoCommand>


class FSM;
class State;
class Transition;

class AddTransitionCommand : public QUndoCommand {
public:
  AddTransitionCommand(FSM *fsm, State *source, State *target,
                       const QString &event, QUndoCommand *parent = nullptr);
  ~AddTransitionCommand();

  void undo() override;
  void redo() override;

private:
  FSM *m_fsm;
  Transition *m_transition;
  State *m_source;
  State *m_target;
  QString m_event;
};

#endif // ADDTRANSITIONCOMMAND_H
