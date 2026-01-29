#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include <QObject>
#include <QString>

class FSM;

/**
 * @brief The JSONSerializer class - Serializes/deserializes FSM to/from JSON
 */
class JSONSerializer : public QObject
{
    Q_OBJECT

public:
    explicit JSONSerializer(QObject *parent = nullptr);
    ~JSONSerializer();

    bool save(const FSM *fsm, const QString &filepath);
    FSM* load(const QString &filepath);
};

#endif // JSONSERIALIZER_H
