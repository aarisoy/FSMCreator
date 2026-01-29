#ifndef DIAGRAMEDITOR_H
#define DIAGRAMEDITOR_H

#include <QGraphicsScene>
#include <QGraphicsView>


class FSM;

class DiagramEditor : public QGraphicsView {
  Q_OBJECT

public:
  explicit DiagramEditor(QWidget *parent = nullptr);
  ~DiagramEditor();

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
  FSM *m_fsm;
  QGraphicsTextItem *m_welcomeText;
  QGraphicsTextItem *m_titleItem;

signals:
  void fsmChanged();
};

#endif // DIAGRAMEDITOR_H
