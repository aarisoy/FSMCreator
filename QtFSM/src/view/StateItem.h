#ifndef STATEITEM_H
#define STATEITEM_H

#include <QGraphicsItem>
#include <QList>
#include <QObject>


class State;
class TransitionItem;

/**
 * @brief The StateItem class represents a visual node on the Graphics Scene
 * corresponding to a @ref State.
 *
 * It handles rendering the circle/oval shape, displaying the state name, and
 * processing mouse input for selection and movement. It maintains a list of
 * connected @ref TransitionItem objects to update them when the state moves.
 *
 * @ingroup View
 */
class StateItem : public QObject, public QGraphicsItem {
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)

public:
  /**
   * @brief Constructs a new StateItem.
   * @param state The model State object this item represents.
   * @param parent The parent graphics item (default: nullptr).
   */
  explicit StateItem(State *state, QGraphicsItem *parent = nullptr);

  /**
   * @brief Destroys the StateItem.
   */
  ~StateItem();

  /**
   * @brief Returns the bounding rectangle for this item.
   * @return QRectF defining the paintable area.
   */
  QRectF boundingRect() const override;

  /**
   * @brief Paints the state item (circle, text, selection highlight).
   * @param painter The painter to use.
   * @param option Style options.
   * @param widget Optional widget argument.
   */
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

  /**
   * @brief Gets the associated model State object.
   * @return Pointer to the State object.
   */
  State *state() const;

  /**
   * @brief Registers a connected TransitionItem.
   * This is used to notify transitions when this state moves so they can
   * redraw.
   * @param item Pointer to the TransitionItem.
   */
  void addTransitionItem(TransitionItem *item);

protected:
  /**
   * @brief Handles property changes (like position changes).
   * @param change The type of change.
   * @param value The new value.
   * @return The reconciled value.
   */
  QVariant itemChange(GraphicsItemChange change,
                      const QVariant &value) override;

  /**
   * @brief Handles double-click events (usually opens properties or rename
   * dialog).
   * @param event The mouse event details.
   */
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
  State *m_state;
  QList<TransitionItem *> m_connectedTransitions;
};

#endif // STATEITEM_H
