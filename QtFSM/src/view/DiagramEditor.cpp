#include "DiagramEditor.h"
#include "../model/FSM.h"
#include "../model/State.h"
#include "../model/Transition.h"
#include "StateItem.h"
#include "TransitionDialog.h"
#include "TransitionItem.h"
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QWheelEvent>

DiagramEditor::DiagramEditor(QWidget *parent)
    : QGraphicsView(parent), m_fsm(nullptr), m_welcomeText(nullptr),
      m_titleItem(nullptr) {
  m_scene = new QGraphicsScene(this);
  setScene(m_scene);

  // Set scene size
  m_scene->setSceneRect(-2000, -2000, 4000, 4000);

  // Enable antialiasing
  setRenderHint(QPainter::Antialiasing);
  setRenderHint(QPainter::TextAntialiasing);
  setRenderHint(QPainter::SmoothPixmapTransform);

  // Enable drag mode
  setDragMode(QGraphicsView::RubberBandDrag);

  // Set background
  setBackgroundBrush(QBrush(QColor(250, 250, 250)));

  // Set viewport update mode for better performance
  setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

  // Enable mouse tracking
  setMouseTracking(true);
}

DiagramEditor::~DiagramEditor() {}

// Helper to rebuild scene from FSM model
void DiagramEditor::rebuildScene() {
  m_scene->clear();
  m_titleItem = nullptr;
  m_welcomeText = nullptr;

  if (!m_fsm)
    return;

  // 1. Create State Items
  QMap<State *, StateItem *> stateItemMap;
  for (State *state : m_fsm->states()) {
    StateItem *item = new StateItem(state);
    m_scene->addItem(item);
    stateItemMap.insert(state, item);
  }

  // 2. Create Transition Items
  for (State *state : m_fsm->states()) {
    for (Transition *t : state->transitions()) {
      StateItem *sourceItem = stateItemMap.value(t->sourceState());
      StateItem *targetItem = stateItemMap.value(t->targetState());

      if (sourceItem && targetItem) {
        TransitionItem *tItem = new TransitionItem(t, sourceItem, targetItem);
        m_scene->addItem(tItem);
        sourceItem->addTransitionItem(tItem);
        targetItem->addTransitionItem(tItem);
      }
    }
  }

  // 3. Create Title
  QFont titleFont("Segoe UI", 24, QFont::Bold);
  m_titleItem = m_scene->addText(m_fsm->name(), titleFont);
  m_titleItem->setDefaultTextColor(QColor(60, 60, 60));
  m_titleItem->setPos(mapToScene(20, 20));
  m_titleItem->setZValue(100);
}

void DiagramEditor::setFSM(FSM *fsm) {
  if (m_fsm != fsm) {
    if (m_fsm) {
      disconnect(m_fsm, &FSM::nameChanged, this, &DiagramEditor::updateTitle);
    }

    m_fsm = fsm;

    if (m_fsm) {
      connect(m_fsm, &FSM::nameChanged, this, &DiagramEditor::updateTitle);
    }

    rebuildScene();
  }
}

void DiagramEditor::updateTitle(const QString &name) {
  if (m_titleItem) {
    m_titleItem->setPlainText(name);
  }
}

FSM *DiagramEditor::fsm() const { return m_fsm; }

void DiagramEditor::addState() {
  if (!m_fsm) {
    m_fsm = new FSM(this);
  }

  // Remove welcome message on first state
  if (m_welcomeText) {
    m_scene->removeItem(m_welcomeText);
    delete m_welcomeText;
    m_welcomeText = nullptr;
  }

  // Create new state with smart naming (recycle numbers)
  int i = 1;
  QString stateName;
  while (true) {
    stateName = QString("State%1").arg(i);
    bool exists = false;
    for (State *s : m_fsm->states()) {
      if (s->name() == stateName) {
        exists = true;
        break;
      }
    }
    if (!exists)
      break;
    i++;
  }

  State *state = new State(stateName, stateName, m_fsm);

  // Position smart placement
  QRectF bounds;
  bool hasStates = false;
  for (State *s : m_fsm->states()) {
    if (!hasStates) {
      bounds = QRectF(s->position(), QSizeF(120, 80)); // Approx size
      hasStates = true;
    } else {
      bounds = bounds.united(QRectF(s->position(), QSizeF(120, 80)));
    }
  }

  QPointF newPos;
  if (!hasStates) {
    // First state: center of view
    newPos = mapToScene(viewport()->rect().center());
  } else {
    // Subsequent states: Right of the current bounds + spacing
    newPos = QPointF(bounds.right() + 50, bounds.top());
  }

  state->setPosition(newPos);

  // Set as initial if it's the first state
  if (m_fsm->states().isEmpty()) {
    state->setInitial(true);
    m_fsm->setInitialState(state);
  }

  // Add to FSM model
  m_fsm->addState(state);

  // Create visual item
  StateItem *stateItem = new StateItem(state);
  m_scene->addItem(stateItem);

  emit fsmChanged();
}

void DiagramEditor::deleteSelected() {
  QList<QGraphicsItem *> selected = m_scene->selectedItems();
  for (QGraphicsItem *item : selected) {
    StateItem *stateItem = dynamic_cast<StateItem *>(item);
    if (stateItem && m_fsm) {
      m_fsm->removeState(stateItem->state());
      m_scene->removeItem(item);
      delete item; // Delete item should be safe after removal
    } else {
      // For transitions or other items
      m_scene->removeItem(item);
      delete item;
    }
  }
}

void DiagramEditor::startTransitionMode() {
  if (!m_fsm || m_fsm->states().count() < 2) {
    QMessageBox::warning(
        this, tr("Not Enough States"),
        tr("You need at least 2 states to create a transition."));
    return;
  }

  TransitionDialog dialog(m_fsm, this);
  if (dialog.exec() == QDialog::Accepted) {
    State *source = dialog.sourceState();
    State *target = dialog.targetState();
    QString eventName = dialog.eventName();

    if (source && target) {
      // Add transition to model
      Transition *transition = new Transition(source, target);
      if (!eventName.isEmpty()) {
        transition->setEvent(eventName);
      }
      source->addTransition(transition);

      // Create Visual
      StateItem *sourceItem = nullptr;
      StateItem *targetItem = nullptr;

      // Find items for states
      for (QGraphicsItem *item : m_scene->items()) {
        StateItem *sItem = dynamic_cast<StateItem *>(item);
        if (sItem) {
          if (sItem->state() == source)
            sourceItem = sItem;
          if (sItem->state() == target)
            targetItem = sItem;
        }
      }

      if (sourceItem && targetItem) {
        TransitionItem *tItem =
            new TransitionItem(transition, sourceItem, targetItem);
        m_scene->addItem(tItem);

        // Register for updates
        sourceItem->addTransitionItem(tItem);
        targetItem->addTransitionItem(tItem);
      }

      // Notify change to update code
      emit fsmChanged();
    }
  }
}

void DiagramEditor::drawBackground(QPainter *painter, const QRectF &rect) {
  // Plain background (color set by setBackgroundBrush)
  painter->fillRect(rect, backgroundBrush());
}

void DiagramEditor::resizeEvent(QResizeEvent *event) {
  QGraphicsView::resizeEvent(event);

  // Keep title at top-left
  if (m_titleItem) {
    m_titleItem->setPos(mapToScene(20, 20));
  }
}

void DiagramEditor::wheelEvent(QWheelEvent *event) {
  // Zoom with mouse wheel
  const double scaleFactor = 1.15;

  if (event->angleDelta().y() > 0) {
    scale(scaleFactor, scaleFactor);
  } else {
    scale(1.0 / scaleFactor, 1.0 / scaleFactor);
  }
}
