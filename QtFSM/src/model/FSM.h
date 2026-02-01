#ifndef FSM_H
#define FSM_H

#include "Event.h"
#include "State.h"
#include "Transition.h"
#include <QList>
#include <QObject>
#include <QString>

/**
 * @brief The FSM class - Main container for a finite state machine
 *
 * This class holds all states, transitions, and events that make up the FSM.
 * It provides validation and management methods for the state machine.
 */
class FSM : public QObject {
  Q_OBJECT

  // Allow commands to access private members for undo/redo
  friend class AddStateCommand;
  friend class DeleteStateCommand;
  friend class AddTransitionCommand;
  friend class DeleteTransitionCommand;

public:
  explicit FSM(QObject *parent = nullptr);
  ~FSM();

  // Basic properties
  QString name() const;
  void setName(const QString &name);

  // Force update signals
  void forceUpdate();

  // State management
  void addState(State *state);
  void removeState(State *state);
  void removeStateWithoutDelete(
      State *state); // For undo/redo - doesn't delete the state
  QList<State *> states() const;
  State *stateById(const QString &id) const;

  // Transition management
  void addTransition(Transition *transition);
  void removeTransition(Transition *transition);
  QList<Transition *> transitions() const;
  Transition *transitionById(const QString &id) const;

  // Event management
  void addEvent(Event *event);
  void removeEvent(Event *event);
  QList<Event *> events() const;

  // Initial state
  State *initialState() const;
  void setInitialState(State *state);

  // Validation
  bool validate(QString *errorMessage = nullptr) const;

  // Clear all
  void clear();

signals:
  void stateAdded(State *state);
  void stateRemoved(State *state);
  void transitionAdded(Transition *transition);
  void transitionRemoved(Transition *transition);
  void nameChanged(const QString &name);
  void modified();

private:
  QString m_name;
  QList<State *> m_states;
  QList<Transition *> m_transitions;
  QList<Event *> m_events;
  State *m_initialState;
};

#endif // FSM_H
