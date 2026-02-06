#ifndef CODEPARSER_H
#define CODEPARSER_H

#include <QString>

class FSM;
class QObject;

/**
 * @brief The CodeParser class represents the high-level interface for parsing
 * config-based C++ code.
 *
 * It parses the strict C++ config format emitted by the CodeGenerator and
 * reconstructs a fully populated @ref FSM object (including UI positions).
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
   * @brief Parses the provided C++ config code to reconstruct an FSM.
   *
   * This method expects the exact layout produced by the config-based
   * CodeGenerator (cfg.initial and StateConfig assignments).
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
