#include "State.h"
#include "Transition.h"
#include <QUuid>

State::State(QObject *parent)
    : QObject(parent), m_id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      m_isInitial(false), m_isFinal(false) {}

State::State(const QString &id, const QString &name, QObject *parent)
    : QObject(parent), m_id(id), m_name(name), m_isInitial(false),
      m_isFinal(false) {}

State::~State() {}

QString State::id() const { return m_id; }

void State::setId(const QString &id) {
  if (m_id != id) {
    m_id = id;
    emit idChanged(m_id);
  }
}

QString State::name() const { return m_name; }

void State::setName(const QString &name) {
  if (m_name != name) {
    m_name = name;
    emit nameChanged(m_name);
  }
}

QString State::entryAction() const { return m_entryAction; }

void State::setEntryAction(const QString &action) {
  if (m_entryAction != action) {
    m_entryAction = action;
    emit entryActionChanged(m_entryAction);
  }
}

QString State::exitAction() const { return m_exitAction; }

void State::setExitAction(const QString &action) {
  if (m_exitAction != action) {
    m_exitAction = action;
    emit exitActionChanged(m_exitAction);
  }
}

QPointF State::position() const { return m_position; }

void State::setPosition(const QPointF &pos) {
  if (m_position != pos) {
    m_position = pos;
    emit positionChanged(m_position);
  }
}

bool State::isInitial() const { return m_isInitial; }

void State::setInitial(bool initial) {
  if (m_isInitial != initial) {
    m_isInitial = initial;
    emit initialChanged(m_isInitial);
  }
}

bool State::isFinal() const { return m_isFinal; }

void State::setFinal(bool final) {
  if (m_isFinal != final) {
    m_isFinal = final;
    emit finalChanged(m_isFinal);
  }
}

QList<Transition *> State::transitions() const { return m_transitions; }

void State::addTransition(Transition *transition) {
  if (!m_transitions.contains(transition)) {
    m_transitions.append(transition);
    // We could emit a signal here if needed
  }
}

void State::removeTransition(Transition *transition) {
  m_transitions.removeAll(transition);
}

QList<QString> State::customFunctions() const { return m_customFunctions; }

void State::addFunction(const QString &functionSignature) {
  if (!m_customFunctions.contains(functionSignature)) {
    m_customFunctions.append(functionSignature);
    emit customFunctionAdded(functionSignature);
  }
}

void State::removeFunction(const QString &functionSignature) {
  if (m_customFunctions.removeOne(functionSignature)) {
    emit customFunctionRemoved(functionSignature);
  }
}
