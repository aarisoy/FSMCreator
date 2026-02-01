#ifndef ADDSTATECOMMAND_H
#define ADDSTATECOMMAND_H

#include <QPointF>
#include <QString>
#include <QUndoCommand>


class FSM;
class State;

/**
 * @brief Command to add a state to the FSM with undo/redo support
 */
class AddStateCommand : public QUndoCommand {
public:
  AddStateCommand(FSM *fsm, const QString &stateId, const QString &stateName,
                  const QPointF &position, QUndoCommand *parent = nullptr);
  ~AddStateCommand();

  void undo() override;
  void redo() override;

private:
  FSM *m_fsm;
  State *m_state;
  QString m_stateId;
  QString m_stateName;
  QPointF m_position;
  bool m_isInitial;
};

#endif // ADDSTATECOMMAND_H
