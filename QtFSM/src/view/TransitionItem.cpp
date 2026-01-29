#include "TransitionItem.h"
#include "StateItem.h"
#include "../model/Transition.h"
#include <QPainter>
#include <QtMath>
#include <QPen>

TransitionItem::TransitionItem(Transition *transition, StateItem *source, StateItem *target, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_transition(transition)
    , m_sourceItem(source)
    , m_targetItem(target)
{
    setZValue(0); // below states
    updatePosition();
}

TransitionItem::~TransitionItem()
{
}

void TransitionItem::updatePosition()
{
    if (!m_sourceItem || !m_targetItem) return;

    prepareGeometryChange();
    
    // Calculate centers
    QPointF sourceCenter = m_sourceItem->scenePos();
    QPointF targetCenter = m_targetItem->scenePos();
    
    QLineF centerLine(sourceCenter, targetCenter);
    
    // Calculate intersection points with borders
    // We map the state's rect to scene to get scene rect
    QRectF sourceRect = m_sourceItem->mapToScene(m_sourceItem->boundingRect()).boundingRect();
    QRectF targetRect = m_targetItem->mapToScene(m_targetItem->boundingRect()).boundingRect();
    
    m_sourcePoint = getIntersectionPoint(centerLine, sourceRect);
    m_targetPoint = getIntersectionPoint(QLineF(targetCenter, sourceCenter), targetRect);
    
    calculatePath();
}

// Helper to find intersection between a line (from center) and a rect
QPointF TransitionItem::getIntersectionPoint(const QLineF &line, const QRectF &rect) const
{
    QPointF intersection;
    
    // Define the 4 lines of the rect
    QList<QLineF> borders;
    borders << QLineF(rect.topLeft(), rect.topRight());
    borders << QLineF(rect.topRight(), rect.bottomRight());
    borders << QLineF(rect.bottomRight(), rect.bottomLeft());
    borders << QLineF(rect.bottomLeft(), rect.topLeft());
    
    QPointF p1 = line.p1();
    QPointF p2 = line.p2();
    QLineF polyLine(p1, p2);

    for (const QLineF &border : borders) {
        // We use BoundedIntersection because we only care if it hits the rect border
        if (polyLine.intersects(border, &intersection) == QLineF::BoundedIntersection) {
            return intersection;
        }
    }
    
    // Fallback if inside or error
    return line.p1();
}

void TransitionItem::calculatePath()
{
    m_path = QPainterPath();
    
    // If self-transition (Source == Target)
    if (m_sourceItem == m_targetItem) {
        QRectF rect = m_sourceItem->mapToScene(m_sourceItem->boundingRect()).boundingRect();
        QPointF topLeft = rect.topLeft();
        
        // Loop on the top-left corner
        m_path.moveTo(rect.right() - 20, rect.top());
        m_path.cubicTo(rect.right() + 40, rect.top() - 60,
                       rect.right() + 80, rect.top() + 40,
                       rect.right(), rect.top() + 20);
        return;
    }
    
    // Straight line between borders
    m_path.moveTo(m_sourcePoint);
    m_path.lineTo(m_targetPoint);
}

QRectF TransitionItem::boundingRect() const
{
    if (m_sourceItem == m_targetItem) {
        // Enforce a large enough rect for self-loops
        QRectF rect = m_sourceItem->mapToScene(m_sourceItem->boundingRect()).boundingRect();
        return QRectF(rect.right() - 20, rect.top() - 60, 120, 120);
    }
    
    // Padding for arrow head and text
    return m_path.boundingRect().adjusted(-50, -50, 50, 50); 
}

void TransitionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    if (!m_sourceItem || !m_targetItem) return;
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    QPen pen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    if (isSelected()) {
        pen.setColor(Qt::blue);
        pen.setWidth(3);
    }
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    
    painter->drawPath(m_path);
    
    // Draw Arrow Head at the end of path (m_targetPoint)
    if (m_path.elementCount() < 2) return;
    
    // Get direction for arrow head
    double angle;
    QPointF endPoint;
    
    if (m_sourceItem == m_targetItem) {
        // For cubic curve, take tangent at end
        double t = 1.0;
        endPoint = m_path.pointAtPercent(t);
        QPointF pBefore = m_path.pointAtPercent(t - 0.05);
        QLineF tangent(pBefore, endPoint);
        angle = std::atan2(-tangent.dy(), tangent.dx());
    } else {
        // Straight line
        QLineF line(m_sourcePoint, m_targetPoint);
        angle = std::atan2(-line.dy(), line.dx());
        endPoint = m_targetPoint;
    }
    
    QPointF arrowP1 = endPoint - QPointF(sin(angle + M_PI / 3) * 10,
                                        cos(angle + M_PI / 3) * 10);
    QPointF arrowP2 = endPoint - QPointF(sin(angle + M_PI - M_PI / 3) * 10,
                                        cos(angle + M_PI - M_PI / 3) * 10);
                                        
    painter->setBrush(isSelected() ? Qt::blue : Qt::black);
    painter->setPen(Qt::NoPen);
    painter->drawPolygon(QPolygonF() << endPoint << arrowP1 << arrowP2);
    
    // Draw Label (Event)
    if (m_transition && !m_transition->event().isEmpty()) {
        QPointF mid = m_path.pointAtPercent(0.5);
        
        // Draw white background for text
        painter->setPen(pen); // Restore pen
        QFont font = painter->font();
        font.setPointSize(9);
        painter->setFont(font);
        
        QFontMetrics fm(font);
        int textWidth = fm.horizontalAdvance(m_transition->event());
        int textHeight = fm.height();
        QRectF textRect(mid.x() - textWidth/2 - 4, mid.y() - textHeight/2 - 4, textWidth + 8, textHeight + 4);
        
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255, 200)); // Semi-transparent white
        painter->drawRoundedRect(textRect, 4, 4);
        
        painter->setPen(Qt::black);
        painter->drawText(mid.x() - textWidth/2, mid.y() + textHeight/2 - fm.descent(), m_transition->event());
    }
}
