#include "AddStateCommand.h"
#include "../../model/FSM.h"
#include "../../model/State.h"
#include <QDebug>

AddStateCommand::AddStateCommand(FSM *fsm, const QString &stateId,
                                 const QString &stateName,
                                 const QPointF &position, QUndoCommand *parent)
    : QUndoCommand(parent), m_fsm(fsm), m_state(nullptr), m_stateId(stateId),
      m_stateName(stateName), m_position(position),
      m_isInitial(fsm->states().isEmpty()) // First state is initial
{
  setText(QObject::tr("Add State '%1'").arg(stateName));
}

AddStateCommand::~AddStateCommand() {
  // Delete the state if it's not in the FSM (was undone and never redone)
  if (m_state && !m_fsm->states().contains(m_state)) {
    m_state->setParent(nullptr); // Remove parent before delete
    delete m_state;
  }
}

void AddStateCommand::redo() {
  qDebug() << "AddStateCommand::redo() - Adding state:" << m_stateName;
  if (!m_state) {
    // First time: create the state with FSM as parent (Qt will manage memory)
    m_state = new State(m_stateId, m_stateName, m_fsm);
    m_state->setPosition(m_position);
    if (m_isInitial) {
      m_state->setInitial(true);
    }
  }

  // Add to FSM (just adds to internal list, doesn't affect Qt parent)
  if (m_isInitial) {
    m_fsm->setInitialState(m_state);
  }
  m_fsm->addState(m_state);
  qDebug() << "AddStateCommand::redo() completed - FSM now has"
           << m_fsm->states().size() << "states";
}

void AddStateCommand::undo() {
  qDebug() << "AddStateCommand::undo() - Removing state:" << m_stateName;
  // Use removeStateWithoutDelete to prevent FSM from deleting the state
  m_fsm->removeStateWithoutDelete(m_state);
  if (m_isInitial) {
    m_fsm->setInitialState(nullptr);
  }
  qDebug() << "AddStateCommand::undo() completed - FSM now has"
           << m_fsm->states().size() << "states";
}
