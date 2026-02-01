#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include <QObject>
#include <QString>

class FSM;

/**
 * @brief The JSONSerializer class - Serializes/deserializes FSM to/from JSON
 */
/**
 * @brief The JSONSerializer class handles persistence of the FSM model using
 * JSON format.
 *
 * It provides methods to save the current FSM state to a file and load it back,
 * ensuring all properties (identifiers, names, layout positions) are preserved.
 *
 * @ingroup Serialization
 */
class JSONSerializer : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Constructs a new JSONSerializer.
   * @param parent The parent QObject.
   */
  explicit JSONSerializer(QObject *parent = nullptr);

  /**
   * @brief Destroys the JSONSerializer.
   */
  ~JSONSerializer();

  /**
   * @brief Saves the given FSM model to a JSON file.
   * @param fsm The FSM model to serialize.
   * @param filepath The absolute path to the destination file.
   * @return true if saving was successful, false otherwise.
   */
  bool save(const FSM *fsm, const QString &filepath);

  /**
   * @brief Loads an FSM model from a JSON file.
   * @param filepath The absolute path to the source file.
   * @return A pointer to the deserialized FSM, or nullptr if loading failed.
   */
  FSM *load(const QString &filepath);
};

#endif // JSONSERIALIZER_H
