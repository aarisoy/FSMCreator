#ifndef TRANSITIONITEM_H
#define TRANSITIONITEM_H

#include <QGraphicsItem>

class Transition;
class StateItem;

/**
 * @brief The TransitionItem class - Visual representation of a transition
 */
class TransitionItem : public QGraphicsItem
{
public:
    explicit TransitionItem(Transition *transition, StateItem *source, StateItem *target, QGraphicsItem *parent = nullptr);
    ~TransitionItem();
    
    // Update position based on attached states
    void updatePosition();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    Transition *m_transition;
    StateItem *m_sourceItem;
    StateItem *m_targetItem;
    QPainterPath m_path;
    QPointF m_sourcePoint;
    QPointF m_targetPoint;
    
    void calculatePath();
    QPointF getIntersectionPoint(const QLineF &line, const QRectF &rect) const;
};

#endif // TRANSITIONITEM_H
