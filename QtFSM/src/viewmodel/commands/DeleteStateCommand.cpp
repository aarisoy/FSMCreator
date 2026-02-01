#include "DeleteStateCommand.h"
#include "../../model/FSM.h"
#include "../../model/State.h"
#include "../../model/Transition.h"

DeleteStateCommand::DeleteStateCommand(FSM *fsm, State *state,
                                       QUndoCommand *parent)
    : QUndoCommand(parent), m_fsm(fsm), m_state(state), m_stateId(state->id()),
      m_stateName(state->name()), m_position(state->position()),
      m_wasInitial(state->isInitial()) {
  setText(QObject::tr("Delete State '%1'").arg(state->name()));

  // Store all transitions connected to this state
  for (Transition *trans : m_fsm->transitions()) {
    if (trans->sourceState() == state || trans->targetState() == state) {
      TransitionData data;
      data.source = trans->sourceState();
      data.target = trans->targetState();
      data.event = trans->event();
      m_connectedTransitions.append(data);
    }
  }
}

DeleteStateCommand::~DeleteStateCommand() {
  // Only delete if the state is not in the FSM (i.e., command was executed)
  if (m_state && !m_fsm->states().contains(m_state)) {
    delete m_state;
  }
}

void DeleteStateCommand::redo() {
  // Remove connected transitions first
  QList<Transition *> transitionsToRemove;
  for (Transition *trans : m_fsm->transitions()) {
    if (trans->sourceState() == m_state || trans->targetState() == m_state) {
      transitionsToRemove.append(trans);
    }
  }
  for (Transition *trans : transitionsToRemove) {
    m_fsm->removeTransition(trans);
  }

  m_fsm->removeStateWithoutDelete(m_state);
  if (m_wasInitial) {
    m_fsm->setInitialState(nullptr);
  }
}

void DeleteStateCommand::undo() {
  // Re-add the state
  m_fsm->addState(m_state);
  if (m_wasInitial) {
    m_fsm->setInitialState(m_state);
  }

  // Restore connected transitions
  for (const TransitionData &data : m_connectedTransitions) {
    Transition *trans = new Transition(data.source, data.target, m_fsm);
    trans->setEvent(data.event);
    m_fsm->addTransition(trans);
  }
}
