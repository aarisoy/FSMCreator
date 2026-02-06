#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <QObject>
#include <QString>

class FSM;

/**
 * @brief The CodeGenerator class is responsible for converting the FSM model
 * into C++ source code.
 *
 * It implements the logic to generate a complete C++ config-driven FSM
 * runtime and the FSMConfig data from the current visual model. The generated
 * output is a single C++ translation unit that contains the config structures,
 * a minimal runtime, and a sample usage snippet.
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

  /**
   * @brief Escapes a string for use in a C++ string literal.
   * @param value The raw string.
   * @return Escaped string without surrounding quotes.
   */
  QString escapeStringLiteral(const QString &value);
};

#endif // CODEGENERATOR_H
