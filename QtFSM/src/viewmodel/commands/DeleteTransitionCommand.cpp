#include "DeleteTransitionCommand.h"
#include "../../model/FSM.h"
#include "../../model/State.h"
#include "../../model/Transition.h"

DeleteTransitionCommand::DeleteTransitionCommand(FSM *fsm,
                                                 Transition *transition,
                                                 QUndoCommand *parent)
    : QUndoCommand(parent), m_fsm(fsm), m_transition(transition),
      m_source(transition->sourceState()), m_target(transition->targetState()),
      m_event(transition->event()) {
  setText(QObject::tr("Delete Transition '%1'").arg(m_event));
}

DeleteTransitionCommand::~DeleteTransitionCommand() {
  if (m_transition && !m_fsm->transitions().contains(m_transition)) {
    delete m_transition;
  }
}

void DeleteTransitionCommand::redo() { m_fsm->removeTransition(m_transition); }

void DeleteTransitionCommand::undo() { m_fsm->addTransition(m_transition); }
