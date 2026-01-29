#ifndef EVENT_H
#define EVENT_H

#include <QObject>
#include <QString>

/**
 * @brief The Event class - Represents an event that can trigger transitions
 * 
 * Events are named triggers that can have parameters for code generation.
 */
class Event : public QObject
{
    Q_OBJECT

public:
    explicit Event(QObject *parent = nullptr);
    explicit Event(const QString &name, QObject *parent = nullptr);
    ~Event();

    // Event name
    QString name() const;
    void setName(const QString &name);

    // Event parameters (for code generation)
    QString parameters() const;
    void setParameters(const QString &parameters);

signals:
    void nameChanged(const QString &name);
    void parametersChanged(const QString &parameters);

private:
    QString m_name;
    QString m_parameters;
};

#endif // EVENT_H
