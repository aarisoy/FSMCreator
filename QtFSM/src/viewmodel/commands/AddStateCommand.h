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
/**
 * @brief The AddStateCommand class implements the Undo/Redo logic for adding a
 * state.
 *
 * When executed (redone), it creates a state and adds it to the FSM.
 * When undone, it removes the state from the FSM.
 *
 * @ingroup Commands
 */
class AddStateCommand : public QUndoCommand {
public:
  /**
   * @brief Constructs an AddStateCommand.
   * @param fsm The FSM model.
   * @param stateId The unique identifier for the state.
   * @param stateName The display name of the state.
   * @param position The position where the state should be placed.
   * @param parent The parent undo command (default: nullptr).
   */
  AddStateCommand(FSM *fsm, const QString &stateId, const QString &stateName,
                  const QPointF &position, QUndoCommand *parent = nullptr);

  /**
   * @brief Destructor.
   */
  ~AddStateCommand();

  /**
   * @brief Removes the state from the FSM.
   */
  void undo() override;

  /**
   * @brief Adds the state to the FSM.
   */
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
