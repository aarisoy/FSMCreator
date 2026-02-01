#ifndef TRANSITION_H
#define TRANSITION_H

#include <QObject>
#include <QString>

class State;

/**
 * @brief The Transition class - Represents a transition between states
 *
 * Contains event, guard condition, and action for the transition.
 */
class Transition : public QObject {
  Q_OBJECT

  // Allow commands to access private members for undo/redo
  friend class AddTransitionCommand;
  friend class DeleteTransitionCommand;
  friend class DeleteStateCommand; // Needs to backup transitions

public:
  explicit Transition(QObject *parent = nullptr);
  explicit Transition(State *source, State *target, QObject *parent = nullptr);
  explicit Transition(const QString &id, State *source, State *target,
                      QObject *parent = nullptr);
  ~Transition();

  // Unique identifier
  QString id() const;
  void setId(const QString &id);

  // Source and target states
  State *sourceState() const;
  void setSourceState(State *state);

  State *targetState() const;
  void setTargetState(State *state);

  // Event that triggers the transition
  QString event() const;
  void setEvent(const QString &event);

  // Guard condition (optional)
  QString guard() const;
  void setGuard(const QString &guard);

  // Action executed during transition (optional)
  QString action() const;
  void setAction(const QString &action);

signals:
  void idChanged(const QString &id);
  void sourceStateChanged(State *state);
  void targetStateChanged(State *state);
  void eventChanged(const QString &event);
  void guardChanged(const QString &guard);
  void actionChanged(const QString &action);

private:
  QString m_id;
  State *m_sourceState;
  State *m_targetState;
  QString m_event;
  QString m_guard;
  QString m_action;
};

#endif // TRANSITION_H
