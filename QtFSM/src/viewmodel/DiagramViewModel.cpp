#include "DiagramViewModel.h"
#include "../model/FSM.h"
#include "../model/State.h"
#include "../model/Transition.h"
#include "commands/AddStateCommand.h"
#include "commands/AddTransitionCommand.h"
#include "commands/DeleteStateCommand.h"
#include "commands/DeleteTransitionCommand.h"
#include <QDebug>


DiagramViewModel::DiagramViewModel(QObject *parent)
    : QObject(parent), m_fsm(nullptr), m_undoStack(new QUndoStack(this)) {
  connect(m_undoStack, &QUndoStack::canUndoChanged, this,
          &DiagramViewModel::undoRedoStateChanged);
  connect(m_undoStack, &QUndoStack::canRedoChanged, this,
          &DiagramViewModel::undoRedoStateChanged);

  // Emit modelChanged whenever undo stack state changes (undo, redo, or new
  // command)
  connect(m_undoStack, &QUndoStack::indexChanged, this,
          &DiagramViewModel::modelChanged);
}

DiagramViewModel::~DiagramViewModel() {}

FSM *DiagramViewModel::fsm() const { return m_fsm; }

void DiagramViewModel::setFSM(FSM *fsm) {
  m_fsm = fsm;
  m_undoStack->clear(); // Clear undo stack when changing FSM
}

// Command execution methods
void DiagramViewModel::addState(const QString &id, const QString &name,
                                const QPointF &position) {
  if (!m_fsm)
    return;

  AddStateCommand *cmd = new AddStateCommand(m_fsm, id, name, position);
  m_undoStack->push(cmd);

  // Find the newly added state and emit signal
  State *state = m_fsm->stateById(id);
  if (state) {
    emit stateAdded(state);
  }
}

void DiagramViewModel::deleteState(State *state) {
  if (!m_fsm || !state)
    return;

  DeleteStateCommand *cmd = new DeleteStateCommand(m_fsm, state);
  m_undoStack->push(cmd);
  emit stateRemoved(state);
}

void DiagramViewModel::addTransition(State *source, State *target,
                                     const QString &event) {
  if (!m_fsm || !source || !target)
    return;

  AddTransitionCommand *cmd =
      new AddTransitionCommand(m_fsm, source, target, event);
  m_undoStack->push(cmd);

  // Find the newly added transition and emit signal
  for (Transition *trans : m_fsm->transitions()) {
    if (trans->sourceState() == source && trans->targetState() == target &&
        trans->event() == event) {
      emit transitionAdded(trans);
      break;
    }
  }
}

void DiagramViewModel::deleteTransition(Transition *transition) {
  if (!m_fsm || !transition)
    return;

  DeleteTransitionCommand *cmd = new DeleteTransitionCommand(m_fsm, transition);
  m_undoStack->push(cmd);
  emit transitionRemoved(transition);
}

// Undo/Redo
void DiagramViewModel::undo() {
  qDebug() << "DiagramViewModel::undo() called - canUndo:"
           << m_undoStack->canUndo();
  m_undoStack->undo();
  qDebug() << "DiagramViewModel::undo() completed";
}

void DiagramViewModel::redo() {
  qDebug() << "DiagramViewModel::redo() called - canRedo:"
           << m_undoStack->canRedo();
  m_undoStack->redo();
  qDebug() << "DiagramViewModel::redo() completed";
}

bool DiagramViewModel::canUndo() const { return m_undoStack->canUndo(); }

bool DiagramViewModel::canRedo() const { return m_undoStack->canRedo(); }

QString DiagramViewModel::undoText() const { return m_undoStack->undoText(); }

QString DiagramViewModel::redoText() const { return m_undoStack->redoText(); }

QUndoStack *DiagramViewModel::undoStack() const { return m_undoStack; }
