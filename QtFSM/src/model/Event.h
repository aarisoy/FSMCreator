#ifndef EVENT_H
#define EVENT_H

#include <QObject>
#include <QString>

/**
 * @brief The Event class represents a signal or trigger within the Finite State
 * Machine.
 *
 * Events are the primary mechanism for driving state transitions. An event
 * consists of a unique name and optional parameters that can be used during
 * code generation to pass data between states.
 *
 * @ingroup Model
 */
class Event : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Constructs a new Event object.
   * @param parent The parent QObject (default: nullptr).
   */
  explicit Event(QObject *parent = nullptr);

  /**
   * @brief Constructs a new Event object with a specified name.
   * @param name The name of the event (e.g., "ButtonPressed", "Timeout").
   * @param parent The parent QObject (default: nullptr).
   */
  explicit Event(const QString &name, QObject *parent = nullptr);

  /**
   * @brief Destroys the Event object.
   */
  ~Event();

  /**
   * @brief Gets the name of the event.
   * @return The current name of the event.
   */
  QString name() const;

  /**
   * @brief Sets the name of the event.
   * @param name The new name for the event.
   * @emit nameChanged
   */
  void setName(const QString &name);

  /**
   * @brief Gets the parameters associated with the event.
   *
   * Parameters are typically stored as a string representation (e.g., "int x,
   * float y") and are primarily used during the code generation phase to create
   * function signatures.
   *
   * @return A string containing the event parameters.
   */
  QString parameters() const;

  /**
   * @brief Sets the parameters for the event.
   * @param parameters The new parameters string.
   * @emit parametersChanged
   */
  void setParameters(const QString &parameters);

signals:
  /**
   * @brief Emitted when the event name changes.
   * @param name The new name.
   */
  void nameChanged(const QString &name);

  /**
   * @brief Emitted when the event parameters change.
   * @param parameters The new parameters.
   */
  void parametersChanged(const QString &parameters);

private:
  QString m_name;       ///< The name of the event.
  QString m_parameters; ///< The parameters of the event (used for codegen).
};

#endif // EVENT_H
