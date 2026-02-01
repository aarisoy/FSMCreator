#ifndef CODEPARSER_H
#define CODEPARSER_H

#include <QString>

class FSM;
class QObject;

/**
 * @brief The CodeParser class represents the high-level interface for parsing
 * C++ code.
 *
 * It acts as a facade, coordinating the `Lexer`, `CppParser`, and
 * `ModelBuilder` to transform raw source code into a fully populated @ref FSM
 * object.
 *
 * @ingroup Parsing
 */
class CodeParser {
public:
  /**
   * @brief Constructs a new CodeParser.
   */
  CodeParser();

  /**
   * @brief Parses the provided C++ source code to reconstruct an FSM.
   *
   * This method tokenizes the code, parses it into an AST, and then
   * builds the FSM model from the AST.
   *
   * @param code The C++ source code string to parse.
   * @param parent The parent QObject for the new FSM (for memory management).
   * @return A pointer to the newly created FSM, or nullptr if parsing failed.
   */
  FSM *parse(const QString &code, QObject *parent = nullptr);

  /**
   * @brief Gets the last error message encountered during parsing.
   * @return A string description of the error, or empty string if success.
   */
  QString lastError() const;

private:
  QString m_lastError;
};

#endif // CODEPARSER_H
