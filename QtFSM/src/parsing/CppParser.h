#pragma once

#include "AST.h"
#include "Lexer.h"
#include <QString>
#include <QVector>


namespace FSMParser {

class CppParser {
public:
  explicit CppParser(const QVector<Token> &tokens);

  // Parse entire source into classes
  QVector<ClassDecl *> parse();

  // Error handling
  QString errorMessage() const { return m_errorMessage; }
  bool hasError() const { return !m_errorMessage.isEmpty(); }

private:
  QVector<Token> m_tokens;
  int m_current = 0;
  QString m_errorMessage;

  // Token navigation
  Token peek() const;
  Token previous() const;
  Token advance();
  bool check(TokenType type) const;
  bool match(TokenType type);
  bool isAtEnd() const;
  Token consume(TokenType type, const QString &message);

  // Error handling
  void error(const QString &message);
  void synchronize();

  // Parsing methods
  ClassDecl *parseClass();
  FunctionDecl *parseFunction();
  Statement *parseStatement();
  IfStatement *parseIfStatement();
  ReturnStatement *parseReturnStatement();

  // Expression parsing
  Expression *parseExpression();
  Expression *parseEquality();
  Expression *parseComparison();
  Expression *parseTerm();
  Expression *parseFactor();
  Expression *parsePostfix();
  Expression *parsePrimary();

  // Helpers
  void skipTo(TokenType type);
  void skipUntilBrace();
};

} // namespace FSMParser
