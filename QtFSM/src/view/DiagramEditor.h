#ifndef DIAGRAMEDITOR_H
#define DIAGRAMEDITOR_H

#include <QGraphicsScene>
#include <QGraphicsView>

class FSM;
class DiagramViewModel;

/**
 * @brief The DiagramEditor class describes the central canvas where FSMs are
 * visualized and edited.
 *
 * It is a specialized QGraphicsView that renders the State Machine via a
 * QGraphicsScene. It handles high-level user interactions such as dragging
 * items, zooming, and context menus, relaying specific actions to the @ref
 * DiagramViewModel.
 *
 * @ingroup View
 */
class DiagramEditor : public QGraphicsView {
  Q_OBJECT

public:
  /**
   * @brief Constructs a new DiagramEditor.
   * @param parent The parent widget.
   */
  explicit DiagramEditor(QWidget *parent = nullptr);

  /**
   * @brief Destroys the DiagramEditor.
   */
  ~DiagramEditor();

  /**
   * @brief Sets the ViewModel that this view should observe and interact with.
   * @param viewModel Pointer to the DiagramViewModel.
   */
  void setViewModel(DiagramViewModel *viewModel);

  /**
   * @brief Gets the current ViewModel.
   * @return Pointer to the DiagramViewModel.
   */
  DiagramViewModel *viewModel() const;

  // Legacy - for backward compatibility
  /**
   * @deprecated Direct FSM access is deprecated. Use ViewModel instead.
   */
  void setFSM(FSM *fsm);
  /**
   * @deprecated Direct FSM access is deprecated. Use ViewModel instead.
   */
  FSM *fsm() const;

  /**
   * @brief Triggers the creation of a new State on the canvas.
   */
  void addState();

  /**
   * @brief Deletes the currently selected items (States or Transitions).
   */
  void deleteSelected();

  /**
   * @brief Enters "Transition Creation Mode" where clicking states connects
   * them.
   */
  void startTransitionMode();

  /**
   * @brief Completely clears and rebuilds the graphical scene from the
   * ViewModel data. Use this when a bulk update (like parsing code) has
   * occurred.
   */
  void rebuildScene();

public slots:
  /**
   * @brief Updates the title displayed on the canvas (usually the FSM name).
   * @param name The new title string.
   */
  void updateTitle(const QString &name);

protected:
  /**
   * @brief Draws the background grid/pattern.
   */
  void drawBackground(QPainter *painter, const QRectF &rect) override;

  /**
   * @brief Handles window resize events.
   */
  void resizeEvent(QResizeEvent *event) override;

  /**
   * @brief Handles mouse wheel events (zooming).
   */
  void wheelEvent(QWheelEvent *event) override;

private:
  QGraphicsScene *m_scene;
  DiagramViewModel *m_viewModel;
  FSM *m_fsm; // Direct access - will be phased out
  QGraphicsTextItem *m_welcomeText;
  QGraphicsTextItem *m_titleItem;

signals:
  /**
   * @brief Emitted when the underlying FSM data changes.
   */
  void fsmChanged();
};

#endif // DIAGRAMEDITOR_H
