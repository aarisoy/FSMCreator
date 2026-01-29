#ifndef STATEITEM_H
#define STATEITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QList>

class State;
class TransitionItem;

/**
 * @brief The StateItem class - Visual representation of a state
 */
class StateItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    explicit StateItem(State *state, QGraphicsItem *parent = nullptr);
    ~StateItem();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
    State* state() const;
    void addTransitionItem(TransitionItem *item);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    State *m_state;
    QList<TransitionItem*> m_connectedTransitions;
};

#endif // STATEITEM_H
