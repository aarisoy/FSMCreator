#ifndef DIAGRAMEDITOR_H
#define DIAGRAMEDITOR_H

#include <QGraphicsScene>
#include <QGraphicsView>

class FSM;
class DiagramViewModel;

class DiagramEditor : public QGraphicsView {
  Q_OBJECT

public:
  explicit DiagramEditor(QWidget *parent = nullptr);
  ~DiagramEditor();

  void setViewModel(DiagramViewModel *viewModel);
  DiagramViewModel *viewModel() const;

  // Legacy - for backward compatibility
  void setFSM(FSM *fsm);
  FSM *fsm() const;

  void addState();
  void deleteSelected();
  void startTransitionMode();
  void rebuildScene();

public slots:
  void updateTitle(const QString &name);

protected:
  void drawBackground(QPainter *painter, const QRectF &rect) override;
  void resizeEvent(QResizeEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  QGraphicsScene *m_scene;
  DiagramViewModel *m_viewModel;
  FSM *m_fsm; // Direct access - will be phased out
  QGraphicsTextItem *m_welcomeText;
  QGraphicsTextItem *m_titleItem;

signals:
  void fsmChanged();
};

#endif // DIAGRAMEDITOR_H
