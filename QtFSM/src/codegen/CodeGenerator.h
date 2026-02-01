#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <QObject>
#include <QString>

class FSM;

/**
 * @brief The CodeGenerator class is responsible for converting the FSM model
 * into C++ source code.
 *
 * It implements the logic to generate a complete C++ implementation of the
 * State Pattern corresponding to the current visual model. It generates both
 * the header and source code (returned as a single string or managed
 * otherwise).
 *
 * @ingroup Codegen
 */
class CodeGenerator : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Constructs a new CodeGenerator.
   * @param parent The parent QObject.
   */
  explicit CodeGenerator(QObject *parent = nullptr);

  /**
   * @brief Destroys the CodeGenerator.
   */
  ~CodeGenerator();

  /**
   * @brief Generates the C++ code for the given FSM.
   * @param fsm The FSM model to translate.
   * @return A QString containing the generated C++ code (header/source combined
   * or main content).
   */
  QString generate(const FSM *fsm);

private:
  /**
   * @brief Sanitizes a string to ensure it is a valid C++ identifier.
   * Removes spaces, special characters, etc.
   * @param name The raw name.
   * @return The sanitized identifier.
   */
  QString sanitizeName(const QString &name);
};

#endif // CODEGENERATOR_H
