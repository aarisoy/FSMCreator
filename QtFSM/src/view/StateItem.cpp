#include "StateItem.h"
#include "StateDialog.h"
#include "TransitionItem.h"
#include "../model/State.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>

StateItem::StateItem(State *state, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_state(state)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setCursor(Qt::OpenHandCursor);
    
    // Connect to state changes
    if (m_state) {
        connect(m_state, &State::nameChanged, this, [this]() { update(); });
        connect(m_state, &State::initialChanged, this, [this]() { update(); });
        connect(m_state, &State::finalChanged, this, [this]() { update(); });
        
        // Set initial position from model
        setPos(m_state->position());
    }
}

StateItem::~StateItem()
{
}

State* StateItem::state() const
{
    return m_state;
}

void StateItem::addTransitionItem(TransitionItem *item)
{
    if (!m_connectedTransitions.contains(item)) {
        m_connectedTransitions.append(item);
    }
}

QRectF StateItem::boundingRect() const
{
    return QRectF(-60, -40, 120, 80);
}

void StateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Determine colors based on state type
    QColor fillColor = Qt::white;
    QColor borderColor = QColor(70, 130, 180); // Steel blue
    qreal borderWidth = 2.0;
    
    if (m_state) {
        if (m_state->isInitial()) {
            fillColor = QColor(200, 255, 200); // Light green for initial
            borderColor = QColor(34, 139, 34); // Forest green
            borderWidth = 3.0;
        } else if (m_state->isFinal()) {
            fillColor = QColor(255, 200, 200); // Light red for final
            borderColor = QColor(220, 20, 60); // Crimson
            borderWidth = 3.0;
        }
    }
    
    // Selection effect
    if (option->state & QStyle::State_Selected) {
        borderColor = QColor(255, 165, 0); // Orange
        borderWidth = 3.5;
    }
    
    // Shadow effect
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 30));
    painter->drawRoundedRect(boundingRect().adjusted(3, 3, 3, 3), 12, 12);
    
    // Main state rectangle
    painter->setPen(QPen(borderColor, borderWidth));
    painter->setBrush(fillColor);
    painter->drawRoundedRect(boundingRect(), 12, 12);
    
    // Draw state name
    if (m_state) {
        painter->setPen(Qt::black);
        QFont font = painter->font();
        font.setPointSize(11);
        font.setBold(true);
        painter->setFont(font);
        
        QString displayName = m_state->name().isEmpty() ? "State" : m_state->name();
        painter->drawText(boundingRect().adjusted(5, -20, -5, 20), 
                         Qt::AlignCenter, displayName);
        
        // Draw initial/final indicators
        QFont smallFont = painter->font();
        smallFont.setPointSize(8);
        smallFont.setBold(false);
        painter->setFont(smallFont);
        
        if (m_state->isInitial()) {
            painter->setPen(QColor(0, 100, 0));
            painter->drawText(boundingRect().adjusted(5, 15, -5, -5), 
                            Qt::AlignCenter, "[Initial]");
        }
        if (m_state->isFinal()) {
            painter->setPen(QColor(150, 0, 0));
            painter->drawText(boundingRect().adjusted(5, 15, -5, -5), 
                            Qt::AlignCenter, "[Final]");
        }
    }
}

QVariant StateItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && m_state) {
        // Update model position when item moves
        m_state->setPosition(value.toPointF());
        // qDebug() << "StateItem moved: " << m_state->name() << " to " << value.toPointF();
        
        // Notify transitions
        for (TransitionItem *item : m_connectedTransitions) {
            item->updatePosition();
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void StateItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    
    if (m_state) {
        // Open property editor dialog
        StateDialog dialog(m_state);
        dialog.exec();
        update(); // Redraw after changes
    }
    
    QGraphicsItem::mouseDoubleClickEvent(event);
}
