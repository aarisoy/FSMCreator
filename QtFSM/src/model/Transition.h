#ifndef TRANSITION_H
#define TRANSITION_H

#include <QObject>
#include <QString>

class State;

/**
 * @brief The Transition class represents a directed connection between two @ref
 * State objects.
 *
 * Transitions define the flow of logic in the Finite State Machine. A
 * transition originates from a Source State and points to a Target State. It
 * occurs when a specific @ref Event is triggered, provided that an optional
 * Guard Condition evaluates to true.
 *
 * @ingroup Model
 */
class Transition : public QObject {
  Q_OBJECT

  // Allow commands to access private members for undo/redo
  friend class AddTransitionCommand;
  friend class DeleteTransitionCommand;
  friend class DeleteStateCommand; // Needs to backup transitions

public:
  /**
   * @brief Constructs a new Transition object.
   * @param parent The parent QObject.
   */
  explicit Transition(QObject *parent = nullptr);

  /**
   * @brief Constructs a new Transition between two states.
   * @param source The state where the transition starts.
   * @param target The state where the transition ends.
   * @param parent The parent QObject.
   */
  explicit Transition(State *source, State *target, QObject *parent = nullptr);

  /**
   * @brief Constructs a new Transition with specific ID between two states.
   * @param id Unique identifier for the transition.
   * @param source The source state.
   * @param target The target state.
   * @param parent The parent QObject.
   */
  explicit Transition(const QString &id, State *source, State *target,
                      QObject *parent = nullptr);

  /**
   * @brief Destroys the Transition object.
   */
  ~Transition();

  /**
   * @brief Gets the unique ID of the transition.
   * @return The unique ID string.
   */
  QString id() const;

  /**
   * @brief Sets the unique ID of the transition.
   * @param id The new unique ID.
   * @emit idChanged
   */
  void setId(const QString &id);

  // =========================================================================
  // Topology
  // =========================================================================

  /**
   * @brief Gets the Source State of this transition.
   * @return Pointer to the source State.
   */
  State *sourceState() const;

  /**
   * @brief Sets the Source State of this transition.
   * @param state Pointer to the new source State.
   * @emit sourceStateChanged
   */
  void setSourceState(State *state);

  /**
   * @brief Gets the Target State of this transition.
   * @return Pointer to the target State.
   */
  State *targetState() const;

  /**
   * @brief Sets the Target State of this transition.
   * @param state Pointer to the new target State.
   * @emit targetStateChanged
   */
  void setTargetState(State *state);

  // =========================================================================
  // Properties
  // =========================================================================

  /**
   * @brief Gets the Event that triggers this transition.
   * @return The name of the triggering event.
   */
  QString event() const;

  /**
   * @brief Sets the Event that triggers this transition.
   * @param event The name of the new triggering event.
   * @emit eventChanged
   */
  void setEvent(const QString &event);

  /**
   * @brief Gets the Guard Condition.
   * A guard is a boolean expression (e.g., "x > 0") that must be true for the
   * transition to occur.
   * @return The guard condition string.
   */
  QString guard() const;

  /**
   * @brief Sets the Guard Condition.
   * @param guard The new guard condition string.
   * @emit guardChanged
   */
  void setGuard(const QString &guard);

  /**
   * @brief Gets the Action executed during the transition.
   * @return The action code/string.
   */
  QString action() const;

  /**
   * @brief Sets the Action executed during the transition.
   * @param action The new action code/string.
   * @emit actionChanged
   */
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
