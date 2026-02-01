#pragma once

#include "Token.h"
#include <QMap>
#include <QString>
#include <QVector>


namespace FSMParser {

class Lexer {
public:
  Lexer(const QString &source);

  // Tokenize entire source
  QVector<Token> tokenize();

  // Get next token (for streaming)
  Token nextToken();

  // Peek at next token without consuming
  Token peekToken();

  // Check if at end
  bool isAtEnd() const { return m_current >= m_source.length(); }

  // Get error message if any
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
