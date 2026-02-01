#pragma once

#include "Token.h"
#include <QMap>
#include <QString>
#include <QVector>

namespace FSMParser {

/**
 * @brief The Lexer class performs lexical analysis (tokenization).
 *
 * It reads the raw C++ source string and breaks it down into meaningful units
 * called Tokens. It handles C++ identifiers, keywords, striung literals, and
 * various operators/symbols.
 *
 * @ingroup Parsing
 */
class Lexer {
public:
  /**
   * @brief Constructs a Lexer for the given source code.
   * @param source The C++ source code string.
   */
  Lexer(const QString &source);

  /**
   * @brief Tokenizes the entire source code at once.
   * @return A vector containing all tokens found in the source.
   */
  QVector<Token> tokenize();

  /**
   * @brief Reads and returns the next token from the stream.
   * Advances the current position.
   * @return The next Token.
   */
  Token nextToken();

  /**
   * @brief Peeks at the next token without advancing the position.
   * @return The lookahead Token.
   */
  Token peekToken();

  /**
   * @brief Checks if the lexer has reached the end of the source.
   * @return true if at end of file (EOF).
   */
  bool isAtEnd() const { return m_current >= m_source.length(); }

  /**
   * @brief Gets the error message if tokenization failed (e.g., unknown char).
   * @return The error string.
   */
  QString errorMessage() const { return m_errorMessage; }

private:
  QString m_source;
  int m_current = 0;
  int m_line = 1;
  int m_column = 1;
  QString m_errorMessage;

  QMap<QString, TokenType> m_keywords;

  void initKeywords();

  // Character inspection
  QChar peek() const;
  QChar peekNext() const;
  QChar advance();
  bool match(QChar expected);

  // Skip helpers
  void skipWhitespace();
  void skipComment();

  // Token scanners
  Token scanIdentifierOrKeyword();
  Token scanStringLiteral();
  Token scanNumber();
  Token makeToken(TokenType type, const QString &value = "");
};

} // namespace FSMParser
