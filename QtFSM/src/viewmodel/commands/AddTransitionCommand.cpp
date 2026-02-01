#include "AddTransitionCommand.h"
#include "../../model/FSM.h"
#include "../../model/State.h"
#include "../../model/Transition.h"

AddTransitionCommand::AddTransitionCommand(FSM *fsm, State *source,
                                           State *target, const QString &event,
                                           QUndoCommand *parent)
    : QUndoCommand(parent), m_fsm(fsm), m_transition(nullptr), m_source(source),
      m_target(target), m_event(event) {
  setText(QObject::tr("Add Transition '%1'").arg(event));
}

AddTransitionCommand::~AddTransitionCommand() {
  if (m_transition && !m_fsm->transitions().contains(m_transition)) {
    delete m_transition;
  }
}

void AddTransitionCommand::redo() {
  if (!m_transition) {
    m_transition = new Transition(m_source, m_target, m_fsm);
    m_transition->setEvent(m_event);
  }
  m_fsm->addTransition(m_transition);
}

void AddTransitionCommand::undo() { m_fsm->removeTransition(m_transition); }
