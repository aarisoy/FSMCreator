#include "Event.h"

Event::Event(QObject *parent)
    : QObject(parent)
{
}

Event::Event(const QString &name, QObject *parent)
    : QObject(parent)
    , m_name(name)
{
}

Event::~Event()
{
}

QString Event::name() const
{
    return m_name;
}

void Event::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged(m_name);
    }
}

QString Event::parameters() const
{
    return m_parameters;
}

void Event::setParameters(const QString &parameters)
{
    if (m_parameters != parameters) {
        m_parameters = parameters;
        emit parametersChanged(m_parameters);
    }
}
