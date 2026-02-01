#ifndef TRANSITIONITEM_H
#define TRANSITIONITEM_H

#include <QGraphicsItem>

class Transition;
class StateItem;

/**
 * @brief The TransitionItem class represents a visual connection/arrow on the
 * Graphics Scene.
 *
 * It connects two @ref StateItem objects. It calculates the optimal path (line
 * or curve) between the source and target items, taking into account their
 * shapes to ensure the arrow terminates exactly at the boundary.
 *
 * @ingroup View
 */
class TransitionItem : public QGraphicsItem {
public:
  /**
   * @brief Constructs a new TransitionItem.
   * @param transition The model Transition object.
   * @param source The visual item for the Source State.
   * @param target The visual item for the Target State.
   * @param parent The parent graphics item.
   */
  explicit TransitionItem(Transition *transition, StateItem *source,
                          StateItem *target, QGraphicsItem *parent = nullptr);

  /**
   * @brief Destroys the TransitionItem.
   */
  ~TransitionItem();

  /**
   * @brief Recalculates the endpoints and path of the arrow.
   * Should be called when either connected StateItem moves.
   */
  void updatePosition();

  /**
   * @brief Gets the underlying Transition model object.
   * @return Pointer to the Transition.
   */
  Transition *transition() const { return m_transition; }

  /**
   * @brief Returns the bounding rectangle for this connection.
   * Includes the arrow path and any decorations (arrowhead).
   * @return QRectF defining the paintable area.
   */
  QRectF boundingRect() const override;

  /**
   * @brief Paints the transition (line, arrowhead, text labels).
   * @param painter The painter to use.
   * @param option Style options.
   * @param widget Optional widget argument.
   */
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

private:
  Transition *m_transition;
  StateItem *m_sourceItem;
  StateItem *m_targetItem;
  QPainterPath m_path;
  QPointF m_sourcePoint;
  QPointF m_targetPoint;

  /**
   * @brief Internal helper to calculate the QPainterPath.
   */
  void calculatePath();

  /**
   * @brief Calculates the intersection point of a line with a rectangle.
   * Used to make sure the arrow stops at the edge of the StateItem.
   * @param line The center-to-center line.
   * @param rect The bounding rect of the state item.
   * @return The intersection point.
   */
  QPointF getIntersectionPoint(const QLineF &line, const QRectF &rect) const;
};

#endif // TRANSITIONITEM_H
