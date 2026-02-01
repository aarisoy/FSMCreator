#ifndef FSM_H
#define FSM_H

#include "Event.h"
#include "State.h"
#include "Transition.h"
#include <QList>
#include <QObject>
#include <QString>

/**
 * @brief The FSM (Finite State Machine) class serves as the root container and
 * manager for the state machine.
 *
 * The FSM class is responsible for managing the lifecycle and relationships of
 * all
 * @ref State, @ref Transition, and @ref Event objects. It acts as the aggregate
 * root of the model layer.
 *
 * Key responsibilities:
 * - Maintenance of the state registry.
 * - Validation of the state machine structure (e.g., ensuring an initial state
 * exists).
 * - Providing lookups for states and transitions by ID.
 *
 * @ingroup Model
 */
class FSM : public QObject {
  Q_OBJECT

  // Allow commands to access private members for undo/redo
  friend class AddStateCommand;
  friend class DeleteStateCommand;
  friend class AddTransitionCommand;
  friend class DeleteTransitionCommand;

public:
  /**
   * @brief Constructs a new FSM object.
   * @param parent The parent QObject (default: nullptr).
   */
  explicit FSM(QObject *parent = nullptr);

  /**
   * @brief Destroys the FSM object and all its children (States, Transitions,
   * Events).
   */
  ~FSM();

  /**
   * @brief Gets the name of the Finite State Machine.
   * @return The name of the FSM.
   */
  QString name() const;

  /**
   * @brief Sets the name of the Finite State Machine.
   * @param name The new name for the FSM.
   * @emit nameChanged
   */
  void setName(const QString &name);

  /**
   * @brief Forces all components to emit their update signals.
   * Useful for refreshing the view when re-attaching listeners.
   */
  void forceUpdate();

  // =========================================================================
  // State Management
  // =========================================================================

  /**
   * @brief Adds a new State to the FSM.
   * The FSM takes ownership of the state object.
   * @param state Pointer to the State to add.
   * @emit stateAdded
   */
  void addState(State *state);

  /**
   * @brief Removes a State from the FSM and deletes it.
   * @param state Pointer to the State to remove.
   * @emit stateRemoved
   */
  void removeState(State *state);

  /**
   * @brief Removes a State from the FSM but DOES NOT delete the object.
   * Primarily used for Undo/Redo operations where the state object serves as a
   * backup.
   * @param state Pointer to the State to remove.
   * @emit stateRemoved
   */
  void removeStateWithoutDelete(State *state);

  /**
   * @brief Retrieves a list of all States in the FSM.
   * @return A list of pointers to State objects.
   */
  QList<State *> states() const;

  /**
   * @brief Finds a State by its unique ID.
   * @param id The unique identifier of the state.
   * @return Pointer to the State if found, otherwise nullptr.
   */
  State *stateById(const QString &id) const;

  // =========================================================================
  // Transition Management
  // =========================================================================

  /**
   * @brief Adds a new Transition to the FSM.
   * The FSM takes ownership of the transition object.
   * @param transition Pointer to the Transition to add.
   * @emit transitionAdded
   */
  void addTransition(Transition *transition);

  /**
   * @brief Removes a Transition from the FSM and deletes it.
   * @param transition Pointer to the Transition to remove.
   * @emit transitionRemoved
   */
  void removeTransition(Transition *transition);

  /**
   * @brief Retrieves a list of all Transitions in the FSM.
   * @return A list of pointers to Transition objects.
   */
  QList<Transition *> transitions() const;

  /**
   * @brief Finds a Transition by its unique ID.
   * @param id The unique identifier of the transition.
   * @return Pointer to the Transition if found, otherwise nullptr.
   */
  Transition *transitionById(const QString &id) const;

  // =========================================================================
  // Event Management
  // =========================================================================

  /**
   * @brief Adds a new Event definition to the FSM.
   * @param event Pointer to the Event to add.
   */
  void addEvent(Event *event);

  /**
   * @brief Removes an Event definition from the FSM.
   * @param event Pointer to the Event to remove.
   */
  void removeEvent(Event *event);

  /**
   * @brief Retrieves a list of all defined Events in the FSM.
   * @return A list of pointers to Event objects.
   */
  QList<Event *> events() const;

  // =========================================================================
  // Lifecycle & Validation
  // =========================================================================

  /**
   * @brief Gets the defined Initial State of the FSM.
   * @return Pointer to the initial State, or nullptr if not set.
   */
  State *initialState() const;

  /**
   * @brief Sets the Initial State of the FSM.
   * @param state Pointer to the state to be marked as initial.
   */
  void setInitialState(State *state);

  /**
   * @brief Validates the integrity of the FSM.
   * Checks for critical errors such as missing initial states or unreachable
   * states.
   * @param errorMessage Optional pointer to a string to receive the error
   * description.
   * @return true if the FSM is valid, false otherwise.
   */
  bool validate(QString *errorMessage = nullptr) const;

  /**
   * @brief Clears the FSM, removing all states, transitions, and events.
   * Resets the FSM to a blank slate.
   */
  void clear();

signals:
  /**
   * @brief Emitted when a new state is added.
   * @param state The added state.
   */
  void stateAdded(State *state);

  /**
   * @brief Emitted when a state is removed.
   * @param state The removed state.
   */
  void stateRemoved(State *state);

  /**
   * @brief Emitted when a new transition is added.
   * @param transition The added transition.
   */
  void transitionAdded(Transition *transition);

  /**
   * @brief Emitted when a transition is removed.
   * @param transition The removed transition.
   */
  void transitionRemoved(Transition *transition);

  /**
   * @brief Emitted when the FSM name changes.
   * @param name The new name.
   */
  void nameChanged(const QString &name);

  /**
   * @brief Emitted when any modification occurs in the FSM structure.
   * Useful for triggering dirty flags or UI updates.
   */
  void modified();

private:
  QString m_name;
  QList<State *> m_states;
  QList<Transition *> m_transitions;
  QList<Event *> m_events;
  State *m_initialState;
};

#endif // FSM_H
