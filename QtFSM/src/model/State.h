#ifndef STATE_H
#define STATE_H

#include <QList>
#include <QObject>
#include <QPointF>
#include <QString>

class Transition;

/**
 * @brief The State class represents a single node/state within the Finite State
 * Machine.
 *
 * Each State has a unique ID, a user-friendly name, and optional code triggers
 * (Entry/Exit actions). Visually, it is represented by a position on the
 * canvas. A state can also be marked as the Initial State (start point) or a
 * Final State (termination point).
 *
 * @ingroup Model
 */
class State : public QObject {
  Q_OBJECT

  // Allow commands to access private members for undo/redo
  friend class AddStateCommand;
  friend class DeleteStateCommand;

public:
  /**
   * @brief Constructs a new State object.
   * @param parent The parent QObject.
   */
  explicit State(QObject *parent = nullptr);

  /**
   * @brief Constructs a new State object with an ID and Name.
   * @param id The unique identifier for the state (e.g., "S1").
   * @param name The display name of the state (e.g., "Idle").
   * @param parent The parent QObject.
   */
  explicit State(const QString &id, const QString &name,
                 QObject *parent = nullptr);

  /**
   * @brief Destroys the State object.
   */
  ~State();

  /**
   * @brief Gets the unique ID of the state.
   * @return The unique ID string.
   */
  QString id() const;

  /**
   * @brief Sets the unique ID of the state.
   * @param id The new unique ID.
   * @emit idChanged
   */
  void setId(const QString &id);

  /**
   * @brief Gets the display name of the state.
   * @return The display name.
   */
  QString name() const;

  /**
   * @brief Sets the display name of the state.
   * @param name The new display name.
   * @emit nameChanged
   */
  void setName(const QString &name);

  // =========================================================================
  // Actions
  // =========================================================================

  /**
   * @brief Gets the Entry Action code/command.
   * @return A string representing the action executed when entering this state.
   */
  QString entryAction() const;

  /**
   * @brief Sets the Entry Action code/command.
   * @param action The new entry action.
   * @emit entryActionChanged
   */
  void setEntryAction(const QString &action);

  /**
   * @brief Gets the Exit Action code/command.
   * @return A string representing the action executed when exiting this state.
   */
  QString exitAction() const;

  /**
   * @brief Sets the Exit Action code/command.
   * @param action The new exit action.
   * @emit exitActionChanged
   */
  void setExitAction(const QString &action);

  // =========================================================================
  // Visual Properties
  // =========================================================================

  /**
   * @brief Gets the visual position of the state on the canvas.
   * @return QPointF representing x,y coordinates.
   */
  QPointF position() const;

  /**
   * @brief Sets the visual position of the state.
   * @param pos The new QPointF coordinates.
   * @emit positionChanged
   */
  void setPosition(const QPointF &pos);

  // =========================================================================
  // State Types
  // =========================================================================

  /**
   * @brief Checks if this is an Initial State.
   * @return true if initial, false otherwise.
   */
  bool isInitial() const;

  /**
   * @brief Marks this state as the Initial State.
   * @param initial true to mark as initial, false to unmark.
   * @emit initialChanged
   */
  void setInitial(bool initial);

  /**
   * @brief Checks if this is a Final State.
   * @return true if final, false otherwise.
   */
  bool isFinal() const;

  /**
   * @brief Marks this state as a Final State.
   * @param final true to mark as final, false to unmark.
   * @emit finalChanged
   */
  void setFinal(bool final);

  // =========================================================================
  // Transitions
  // =========================================================================

  /**
   * @brief Retrieves a list of outgoing transitions from this state.
   * @return List of pointers to Transition objects.
   */
  QList<Transition *> transitions() const;

  /**
   * @brief Adds an outgoing transition to this state.
   * @param transition Pointer to the transition to add.
   */
  void addTransition(Transition *transition);

  /**
   * @brief Removes an outgoing transition from this state.
   * @param transition Pointer to the transition to remove.
   */
  void removeTransition(Transition *transition);

  // =========================================================================
  // Custom Functionalities
  // =========================================================================

  /**
   * @brief Retrieves a list of custom member functions defined in this state.
   * @return List of function signatures (e.g. "void calculate(int x)").
   */
  QList<QString> customFunctions() const;

  /**
   * @brief Adds a custom function to this state.
   * @param functionSignature The full function signature.
   */
  void addFunction(const QString &functionSignature);

  /**
   * @brief Removes a custom function from this state.
   * @param functionSignature The function signature to remove.
   */
  void removeFunction(const QString &functionSignature);

signals:
  void idChanged(const QString &id);
  void nameChanged(const QString &name);
  void entryActionChanged(const QString &action);
  void exitActionChanged(const QString &action);
  void positionChanged(const QPointF &pos);
  void initialChanged(bool initial);
  void finalChanged(bool final);

private:
  QString m_id;
  QString m_name;
  QString m_entryAction;
  QString m_exitAction;
  QPointF m_position;
  bool m_isInitial;
  bool m_isFinal;
  QList<Transition *> m_transitions;
  QList<QString> m_customFunctions;

signals:
  void customFunctionAdded(const QString &function);
  void customFunctionRemoved(const QString &function);
};

#endif // STATE_H
