#ifndef STATE_H
#define STATE_H

#include <QObject>
#include <QString>
#include <QPointF>
#include <QList>

class Transition;

/**
 * @brief The State class - Represents a single state in the FSM
 * 
 * Contains all properties of a state including position (for diagram),
 * entry/exit actions, and state type (initial/final).
 */
class State : public QObject
{
    Q_OBJECT

public:
    explicit State(QObject *parent = nullptr);
    explicit State(const QString &id, const QString &name, QObject *parent = nullptr);
    ~State();

    // Unique identifier
    QString id() const;
    void setId(const QString &id);

    // Display name
    QString name() const;
    void setName(const QString &name);

    // Actions
    QString entryAction() const;
    void setEntryAction(const QString &action);
    
    QString exitAction() const;
    void setExitAction(const QString &action);

    // Position in diagram
    QPointF position() const;
    void setPosition(const QPointF &pos);

    // State type
    bool isInitial() const;
    void setInitial(bool initial);
    
    bool isFinal() const;
    void setFinal(bool final);

    // Transitions
    QList<Transition*> transitions() const;
    void addTransition(Transition *transition);
    void removeTransition(Transition *transition);

signals:
    void idChanged(const QString &id);
    void nameChanged(const QString &name);
    void entryActionChanged(const QString &action);
    void exitActionChanged(const QString &action);
    void positionChanged(const QPointF &pos);
    void initialChanged(bool initial);
    void finalChanged(bool final);

private:
    QString m_id;
    QString m_name;
    QString m_entryAction;
    QString m_exitAction;
    QPointF m_position;
    bool m_isInitial;
    bool m_isFinal;
    QList<Transition*> m_transitions;
};

#endif // STATE_H
