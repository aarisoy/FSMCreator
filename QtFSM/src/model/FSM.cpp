#include "FSM.h"
#include <QSet>

FSM::FSM(QObject *parent) : QObject(parent), m_initialState(nullptr) {}

FSM::~FSM() { clear(); }

QString FSM::name() const { return m_name; }

void FSM::setName(const QString &name) {
  if (m_name != name) {
    m_name = name;
    emit nameChanged(m_name);
    emit modified();
  }
}

void FSM::forceUpdate() {
  emit nameChanged(m_name);
  emit modified();
}

void FSM::addState(State *state) {
  if (state && !m_states.contains(state)) {
    m_states.append(state);
    state->setParent(this);
    emit stateAdded(state);
    emit modified();
  }
}

void FSM::removeState(State *state) {
  if (!m_states.contains(state)) {
    return;
  }

  // CRITICAL FIX: Remove all transitions referencing this state
  // to prevent dangling pointers
  QList<Transition *> transitionsToRemove;
  for (Transition *trans : m_transitions) {
    if (trans->sourceState() == state || trans->targetState() == state) {
      transitionsToRemove.append(trans);
    }
  }

  // Remove collected transitions
  for (Transition *trans : transitionsToRemove) {
    removeTransition(trans);
  }

  // Now safe to remove the state
  if (m_states.removeOne(state)) {
    if (m_initialState == state) {
      m_initialState = nullptr;
    }
    emit stateRemoved(state);
    emit modified();
    state->deleteLater();
  }
}

void FSM::removeStateWithoutDelete(State *state) {
  // Same as removeState but doesn't delete - for undo/redo commands
  if (!m_states.contains(state)) {
    return;
  }

  // CRITICAL FIX: Remove all transitions referencing this state
  QList<Transition *> transitionsToRemove;
  for (Transition *trans : m_transitions) {
    if (trans->sourceState() == state || trans->targetState() == state) {
      transitionsToRemove.append(trans);
    }
  }

  // Remove collected transitions (but don't delete them - command owns them)
  for (Transition *trans : transitionsToRemove) {
    m_transitions.removeOne(trans);
    if (trans->sourceState()) {
      trans->sourceState()->removeTransition(trans);
    }
    emit transitionRemoved(trans);
  }

  // Remove the state
  if (m_states.removeOne(state)) {
    if (m_initialState == state) {
      m_initialState = nullptr;
    }
    emit stateRemoved(state);
    emit modified();
    // Don't call deleteLater() - command owns the state
  }
}

QList<State *> FSM::states() const { return m_states; }

State *FSM::stateById(const QString &id) const {
  for (State *state : m_states) {
    if (state->id() == id) {
      return state;
    }
  }
  return nullptr;
}

void FSM::addTransition(Transition *transition) {
  if (transition && !m_transitions.contains(transition)) {
    m_transitions.append(transition);
    transition->setParent(this);
    if (transition->sourceState()) {
      transition->sourceState()->addTransition(transition);
    }
    emit transitionAdded(transition);
    emit modified();
  }
}

void FSM::removeTransition(Transition *transition) {
  if (m_transitions.removeOne(transition)) {
    if (transition->sourceState()) {
      transition->sourceState()->removeTransition(transition);
    }
    emit transitionRemoved(transition);
    emit modified();
    transition->deleteLater();
  }
}

QList<Transition *> FSM::transitions() const { return m_transitions; }

Transition *FSM::transitionById(const QString &id) const {
  for (Transition *transition : m_transitions) {
    if (transition->id() == id) {
      return transition;
    }
  }
  return nullptr;
}

void FSM::addEvent(Event *event) {
  if (event && !m_events.contains(event)) {
    m_events.append(event);
    event->setParent(this);
    emit modified();
  }
}

void FSM::removeEvent(Event *event) {
  if (m_events.removeOne(event)) {
    emit modified();
    event->deleteLater();
  }
}

QList<Event *> FSM::events() const { return m_events; }

State *FSM::initialState() const { return m_initialState; }

void FSM::setInitialState(State *state) {
  if (m_states.contains(state) && m_initialState != state) {
    m_initialState = state;
    emit modified();
  }
}

bool FSM::validate(QString *errorMessage) const {
  // Must have at least one state
  if (m_states.isEmpty()) {
    if (errorMessage) {
      *errorMessage = "FSM must have at least one state";
    }
    return false;
  }

  // Must have an initial state
  if (!m_initialState) {
    if (errorMessage) {
      *errorMessage = "FSM must have an initial state";
    }
    return false;
  }

  // Check for duplicate State IDs
  QSet<QString> stateIds;
  for (const State *state : m_states) {
    if (stateIds.contains(state->id())) {
      if (errorMessage) {
        *errorMessage =
            QString("Duplicate state ID found: %1").arg(state->id());
      }
      return false;
    }
    stateIds.insert(state->id());
  }

  // Check for duplicate Transition IDs
  QSet<QString> transitionIds;
  for (const Transition *transition : m_transitions) {
    if (transitionIds.contains(transition->id())) {
      if (errorMessage) {
        *errorMessage =
            QString("Duplicate transition ID found: %1").arg(transition->id());
      }
      return false;
    }
    transitionIds.insert(transition->id());
  }

  // Validate all transitions (orphans)
  for (const Transition *transition : m_transitions) {
    if (!transition->sourceState() || !transition->targetState()) {
      if (errorMessage) {
        *errorMessage = QString("Transition %1 has invalid source or target")
                            .arg(transition->id());
      }
      return false;
    }
  }

  // Reachability Check (BFS)
  QSet<State *> reachable;
  QList<State *> queue;

  if (m_initialState) {
    queue.append(m_initialState);
    reachable.insert(m_initialState);
  }

  while (!queue.isEmpty()) {
    State *current = queue.takeFirst();

    // Find neighbors (states connected by outgoing transitions)
    for (const Transition *trans : m_transitions) {
      if (trans->sourceState() == current) {
        State *next = trans->targetState();
        if (next && !reachable.contains(next)) {
          reachable.insert(next);
          queue.append(next);
        }
      }
    }
  }

  // Check if any state is unreachable
  for (State *state : m_states) {
    if (!reachable.contains(state)) {
      if (errorMessage) {
        *errorMessage =
            QString("State '%1' is unreachable from the initial state")
                .arg(state->name());
      }
      return false;
    }
  }

  return true;
}

void FSM::clear() {
  // Clear all transitions
  qDeleteAll(m_transitions);
  m_transitions.clear();

  // Clear all events
  qDeleteAll(m_events);
  m_events.clear();

  // Clear all states
  qDeleteAll(m_states);
  m_states.clear();

  m_initialState = nullptr;
  emit modified();
}
