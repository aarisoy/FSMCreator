#include "CppParser.h"
#include <QDebug>

namespace FSMParser {

CppParser::CppParser(const QVector<Token> &tokens) : m_tokens(tokens) {}

Token CppParser::peek() const {
  if (m_current >= m_tokens.size()) {
    return Token(TokenType::EndOfFile);
  }
  return m_tokens[m_current];
}

Token CppParser::previous() const {
  if (m_current <= 0)
    return Token();
  return m_tokens[m_current - 1];
}

Token CppParser::advance() {
  if (!isAtEnd())
    m_current++;
  return previous();
}

bool CppParser::check(TokenType type) const {
  if (isAtEnd())
    return false;
  return peek().type == type;
}

bool CppParser::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

bool CppParser::isAtEnd() const { return peek().type == TokenType::EndOfFile; }

Token CppParser::consume(TokenType type, const QString &message) {
  if (check(type))
    return advance();
  error(message);
  return Token();
}

void CppParser::error(const QString &message) {
  Token token = peek();
  m_errorMessage = QString("Parse error at line %1: %2 (got '%3')")
                       .arg(token.line)
                       .arg(message)
                       .arg(token.value);
}

void CppParser::synchronize() {
  advance();
  while (!isAtEnd()) {
    if (previous().type == TokenType::Semicolon)
      return;
    if (peek().type == TokenType::RightBrace)
      return;
    if (peek().type == TokenType::Keyword_Class)
      return;
    advance();
  }
}

QVector<ClassDecl *> CppParser::parse() {
  QVector<ClassDecl *> classes;

  while (!isAtEnd()) {
    // Skip until we find 'class'
    if (match(TokenType::Keyword_Class)) {
      ClassDecl *classDecl = parseClass();
      if (classDecl) {
        classes.append(classDecl);
      }
    } else {
      advance(); // Skip other tokens
    }
  }

  return classes;
}

ClassDecl *CppParser::parseClass() {
  // class ClassName
  Token nameToken = consume(TokenType::Identifier, "Expected class name");
  if (hasError())
    return nullptr;

  ClassDecl *classDecl = new ClassDecl(nameToken.value);

  // Check for forward declaration: class Name;
  if (match(TokenType::Semicolon)) {
    // Forward declaration, skip it
    delete classDecl;
    return nullptr;
  }

  // Optional: : public BaseClass
  if (match(TokenType::Colon)) {
    match(TokenType::Keyword_Public); // Skip access specifier
    Token baseToken =
        consume(TokenType::Identifier, "Expected base class name");
    classDecl->baseClass = baseToken.value;
  }

  // {
  consume(TokenType::LeftBrace, "Expected '{' after class name");

  // Check if this is an FSM state class - only parse those in detail
  bool isFSMState =
      (classDecl->baseClass == "MyFSMStateBase" ||
       classDecl->baseClass == "State" ||
       (classDecl->name.endsWith("State") && classDecl->name != "BaseState"));

  if (!isFSMState) {
    // Skip non-FSM classes by consuming tokens until the closing brace
    int braceCount = 1;
    while (braceCount > 0 && !isAtEnd()) {
      if (match(TokenType::LeftBrace)) {
        braceCount++;
      } else if (match(TokenType::RightBrace)) {
        braceCount--;
      } else {
        advance();
      }
    }
    delete classDecl;
    return nullptr;
  }

  // Parse class body (only for FSM state classes)
  while (!check(TokenType::RightBrace) && !isAtEnd()) {
    // Skip access specifiers
    if (match(TokenType::Keyword_Public) || match(TokenType::Keyword_Private) ||
        match(TokenType::Keyword_Protected)) {
      consume(TokenType::Colon, "Expected ':' after access specifier");
      continue;
    }

    // Skip virtual keyword
    bool isVirtual = match(TokenType::Keyword_Virtual);

    // Skip destructors (~ClassName)
    if (check(TokenType::Tilde)) {
      // This is a destructor, skip it
      while (!check(TokenType::Semicolon) && !check(TokenType::LeftBrace) &&
             !isAtEnd()) {
        advance();
      }

      if (check(TokenType::LeftBrace)) {
        // Destructor with body, use brace counting
        int braceCount = 0;
        do {
          if (check(TokenType::LeftBrace))
            braceCount++;
          if (check(TokenType::RightBrace))
            braceCount--;
          advance();
        } while (braceCount > 0 && !isAtEnd());
      }

      match(TokenType::Semicolon);
      continue;
    }

    // Try to parse function
    if (check(TokenType::Identifier) || check(TokenType::Keyword_Void)) {
      // Peek ahead to see if this looks like a function
      int saved = m_current;
      Token first = advance(); //  Return type or constructor name

      // Skip pointers/refs in return type
      while (check(TokenType::Star) || check(TokenType::Ampersand)) {
        advance();
      }

      // Check if next token is identifier (function name) or ( (constructor)
      if (check(TokenType::Identifier)) {
        advance(); // Consume function name to peek at next token

        if (check(TokenType::LeftParen)) {
          // It IS a function!
          m_current = saved;
          FunctionDecl *func = parseFunction();
          if (func) {
            func->isVirtual = isVirtual;
            classDecl->methods.append(func);
          }
        } else {
          // Not a function (e.g. member variable: Type* name;)
          qDebug() << "parseClass: Not a function (no LeftParen), skipping "
                      "member variable";
          // Skip until semicolon
          m_current = saved;
          while (!check(TokenType::Semicolon) &&
                 !check(TokenType::RightBrace) && !isAtEnd()) {
            advance();
          }
          match(TokenType::Semicolon);
        }
      } else if (check(TokenType::LeftParen) &&
                 first.value == classDecl->name) {
        // Constructor - only if the identifier matches the class name
        qDebug() << "parseClass: Constructor detected, skipping";
        m_current = saved;
        while (!check(TokenType::Semicolon) && !check(TokenType::RightBrace) &&
               !isAtEnd()) {
          if (check(TokenType::LeftBrace)) {
            // Constructor with body
            int braceCount = 0;
            do {
              if (check(TokenType::LeftBrace))
                braceCount++;
              if (check(TokenType::RightBrace))
                braceCount--;
              advance();
            } while (braceCount > 0 && !isAtEnd());
            break;
          }
          advance();
        }
        match(TokenType::Semicolon);
      } else {
        // Unknown, skip
        m_current = saved;
        advance();
      }
    } else {
      advance(); // Skip unknown tokens
    }
  }

  // }
  consume(TokenType::RightBrace, "Expected '}' after class body");
  match(TokenType::Semicolon); // Optional semicolon

  return classDecl;
}

FunctionDecl *CppParser::parseFunction() {
  // Return type
  Token returnType = advance();

  // Consume pointers/refs
  while (match(TokenType::Star) || match(TokenType::Ampersand)) {
    // Just skip them for now, we only need the base return type string or just
    // know it's a function
  }

  // Function name
  Token funcName = consume(TokenType::Identifier, "Expected function name");
  if (hasError())
    return nullptr;

  FunctionDecl *func = new FunctionDecl(returnType.value, funcName.value);

  // (
  consume(TokenType::LeftParen, "Expected '(' after function name");

  // Parameters - simplified, just skip until )
  while (!check(TokenType::RightParen) && !isAtEnd()) {
    advance();
  }

  // )
  consume(TokenType::RightParen, "Expected ')' after parameters");

  // Optional const
  match(TokenType::Keyword_Const);

  // Optional override
  if (match(TokenType::Keyword_Override)) {
    func->isOverride = true;
  }

  // {
  if (!match(TokenType::LeftBrace)) {
    // Function declaration without body
    match(TokenType::Semicolon);
    delete func;
    return nullptr;
  }

  qDebug() << "parseFunction: Body loop start at" << m_current
           << peek().typeName();

  // Parse function body
  while (!check(TokenType::RightBrace) && !isAtEnd()) {
    qDebug() << "parseFunction: calling parseStatement at token" << m_current;

    Statement *stmt = parseStatement();
    if (stmt) {
      func->body.append(stmt);
    }
  }

  // }
  consume(TokenType::RightBrace, "Expected '}' after function body");

  return func;
}

Statement *CppParser::parseStatement() {
  if (match(TokenType::Keyword_If)) {
    return parseIfStatement();
  }
  if (match(TokenType::Keyword_Return)) {
    return parseReturnStatement();
  }

  // Skip other statements
  // log skipped token

  while (!check(TokenType::Semicolon) && !check(TokenType::RightBrace) &&
         !isAtEnd()) {
    advance();
  }
  match(TokenType::Semicolon);

  return nullptr;
}

IfStatement *CppParser::parseIfStatement() {
  // (
  consume(TokenType::LeftParen, "Expected '(' after 'if'");

  // condition
  Expression *condition = parseExpression();

  // )
  consume(TokenType::RightParen, "Expected ')' after condition");

  IfStatement *ifStmt = new IfStatement(condition);

  // {
  consume(TokenType::LeftBrace, "Expected '{' after if condition");

  // Parse then block
  while (!check(TokenType::RightBrace) && !isAtEnd()) {
    Statement *stmt = parseStatement();
    if (stmt) {
      ifStmt->thenBlock.append(stmt);
    }
  }

  // }
  consume(TokenType::RightBrace, "Expected '}' after if body");

  return ifStmt;
}

ReturnStatement *CppParser::parseReturnStatement() {
  Expression *value = nullptr;

  if (!check(TokenType::Semicolon)) {
    value = parseExpression();
  }

  consume(TokenType::Semicolon, "Expected ';' after return");

  return new ReturnStatement(value);
}

Expression *CppParser::parseExpression() { return parseEquality(); }

Expression *CppParser::parseEquality() {
  Expression *expr = parseComparison();

  while (match(TokenType::EqualEqual) || match(TokenType::ExclaimEqual)) {
    QString op = previous().value;
    Expression *right = parseComparison();
    expr = new BinaryExpr(expr, op, right);
  }

  return expr;
}

Expression *CppParser::parseComparison() { return parseTerm(); }

Expression *CppParser::parseTerm() { return parseFactor(); }

Expression *CppParser::parseFactor() { return parsePostfix(); }

Expression *CppParser::parsePostfix() {
  Expression *expr = parsePrimary();

  while (true) {
    if (match(TokenType::Dot)) {
      Token member = consume(TokenType::Identifier, "Expected member name");
      expr = new MemberAccessExpr(expr, member.value);
    } else {
      break;
    }
  }

  return expr;
}

Expression *CppParser::parsePrimary() {
  if (match(TokenType::Keyword_New)) {
    Token typeName =
        consume(TokenType::Identifier, "Expected type name after 'new'");
    // Skip constructor args
    if (match(TokenType::LeftParen)) {
      while (!check(TokenType::RightParen) && !isAtEnd())
        advance();
      consume(TokenType::RightParen, "Expected ')' after constructor args");
    }
    return new NewExpr(typeName.value);
  }

  if (match(TokenType::StringLiteral)) {
    return new StringLiteralExpr(previous().value);
  }

  if (match(TokenType::Identifier)) {
    return new IdentifierExpr(previous().value);
  }

  // Unknown - skip
  advance();
  return new IdentifierExpr("unknown");
}

} // namespace FSMParser
