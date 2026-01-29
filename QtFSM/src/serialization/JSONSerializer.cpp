#include "JSONSerializer.h"
#include "../model/FSM.h"

JSONSerializer::JSONSerializer(QObject *parent)
    : QObject(parent)
{
}

JSONSerializer::~JSONSerializer()
{
}

bool JSONSerializer::save(const FSM *fsm, const QString &filepath)
{
    Q_UNUSED(fsm);
    Q_UNUSED(filepath);
    // TODO: Implement JSON serialization
    return false;
}

FSM* JSONSerializer::load(const QString &filepath)
{
    Q_UNUSED(filepath);
    // TODO: Implement JSON deserialization
    return nullptr;
}
