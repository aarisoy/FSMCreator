#pragma once

#include "AST.h"
#include "Lexer.h"
#include <QString>
#include <QVector>

namespace FSMParser {

/**
 * @brief The CppParser class performs syntactic analysis (parsing) on a stream
 * of tokens.
 *
 * It implements a recursive descent parser to construct an Abstract Syntax Tree
 * (AST) representing the C++ code. It specifically looks for patterns relevant
 * to the State Pattern implementation (classes inheriting from State,
 * transition logic).
 *
 * @ingroup Parsing
 */
class CppParser {
public:
  /**
   * @brief Constructs a parser with the given token stream.
   * @param tokens The vector of tokens produced by the Lexer.
   */
  explicit CppParser(const QVector<Token> &tokens);

  /**
   * @brief Parses the token stream into a list of class declarations.
   * This is the main entry point for the parsing process.
   * @return A vector of pointers to ClassDecl AST nodes.
   */
  QVector<ClassDecl *> parse();

  /**
   * @brief Gets the error message if parsing failed.
   * @return The error string.
   */
  QString errorMessage() const { return m_errorMessage; }

  /**
   * @brief Checks if the parser encountered an error.
   * @return true if an error occurred, false otherwise.
   */
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
  QString parseQualifiedType();
};

} // namespace FSMParser
