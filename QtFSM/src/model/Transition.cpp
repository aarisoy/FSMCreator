#include "Transition.h"
#include "State.h"
#include <QUuid>

Transition::Transition(QObject *parent)
    : QObject(parent)
    , m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_sourceState(nullptr)
    , m_targetState(nullptr)
{
}

Transition::Transition(State *source, State *target, QObject *parent)
    : QObject(parent)
    , m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_sourceState(source)
    , m_targetState(target)
{
}

Transition::Transition(const QString &id, State *source, State *target, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_sourceState(source)
    , m_targetState(target)
{
}

Transition::~Transition()
{
}

QString Transition::id() const
{
    return m_id;
}

void Transition::setId(const QString &id)
{
    if (m_id != id) {
        m_id = id;
        emit idChanged(m_id);
    }
}

State* Transition::sourceState() const
{
    return m_sourceState;
}

void Transition::setSourceState(State *state)
{
    if (m_sourceState != state) {
        m_sourceState = state;
        emit sourceStateChanged(m_sourceState);
    }
}

State* Transition::targetState() const
{
    return m_targetState;
}

void Transition::setTargetState(State *state)
{
    if (m_targetState != state) {
        m_targetState = state;
        emit targetStateChanged(m_targetState);
    }
}

QString Transition::event() const
{
    return m_event;
}

void Transition::setEvent(const QString &event)
{
    if (m_event != event) {
        m_event = event;
        emit eventChanged(m_event);
    }
}

QString Transition::guard() const
{
    return m_guard;
}

void Transition::setGuard(const QString &guard)
{
    if (m_guard != guard) {
        m_guard = guard;
        emit guardChanged(m_guard);
    }
}

QString Transition::action() const
{
    return m_action;
}

void Transition::setAction(const QString &action)
{
    if (m_action != action) {
        m_action = action;
        emit actionChanged(m_action);
    }
}
