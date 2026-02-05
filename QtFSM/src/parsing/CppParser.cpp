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
       classDecl->baseClass == "State" || classDecl->name == "MyFSMStateBase" ||
       classDecl->name.endsWith("StateBase") ||
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

    if (match(TokenType::Keyword_Enum)) {
      skipEnumDeclaration();
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

QString CppParser::parseQualifiedType() {
  QString result;

  // Handle const before type (e.g., const std::string)
  if (match(TokenType::Keyword_Const)) {
    result = "const ";
  }

  if (match(TokenType::Keyword_Auto)) {
    result += "auto";
  } else {
  // Handle qualified names: std::string, MyNamespace::MyClass, etc.
  if (check(TokenType::Identifier)) {
    result += advance().value;

    // Handle scope resolution: ::
    while (match(TokenType::DoubleColon)) {
      result += "::";
      if (check(TokenType::Identifier)) {
        result += advance().value;
      }
    }
  } else if (check(TokenType::Keyword_Void)) {
    result += advance().value;
  }
  }

  // Handle pointers and references
  while (check(TokenType::Star) || check(TokenType::Ampersand)) {
    result += advance().value;
  }

  // Handle const after type (e.g., MyClass* const)
  if (match(TokenType::Keyword_Const)) {
    result += " const";
  }

  return result;
}

FunctionDecl *CppParser::parseFunction() {
  // Parse qualified return type (handles std::string, MyClass*, etc.)
  QString returnType = parseQualifiedType();

  // Function name - can be simple (myFunc) or qualified (MyClass::myFunc)
  QString funcName;

  if (check(TokenType::Identifier)) {
    funcName = advance().value;

    // Check for qualified function name (e.g., MyClass::myMethod)
    while (match(TokenType::DoubleColon)) {
      funcName += "::";
      if (check(TokenType::Identifier)) {
        funcName += advance().value;
      }
    }
  }

  if (funcName.isEmpty()) {
    error("Expected function name");
    return nullptr;
  }

  FunctionDecl *func = new FunctionDecl(returnType, funcName);

  // (
  consume(TokenType::LeftParen, "Expected '(' after function name");

  // Parameters
  while (!check(TokenType::RightParen) && !isAtEnd()) {
    // Save position in case we need to backtrack
    int savedPos = m_current;

    // Parse the qualified type (handles std::string, const MyClass&, etc.)
    QString typeStr = parseQualifiedType();

    Parameter param;

    // Check if there's a parameter name after the type
    if (check(TokenType::Identifier)) {
      // We have both type and name: Type name
      param.type = typeStr;
      param.name = advance().value;
    } else if (!typeStr.isEmpty()) {
      // Only type, no name (unnamed parameter)
      param.type = typeStr;
      // param.name remains empty
    } else {
      // Error: couldn't parse type or name, skip to next comma or closing paren
      while (!check(TokenType::Comma) && !check(TokenType::RightParen) &&
             !isAtEnd()) {
        advance();
      }
    }

    if (!param.type.isEmpty()) {
      func->parameters.append(param);
    }

    if (match(TokenType::Comma)) {
      continue;
    }
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

  if (match(TokenType::Keyword_Else)) {
    if (match(TokenType::Keyword_If)) {
      IfStatement *elseIf = parseIfStatement();
      if (elseIf) {
        ifStmt->elseBlock.append(elseIf);
      }
    } else if (match(TokenType::LeftBrace)) {
      while (!check(TokenType::RightBrace) && !isAtEnd()) {
        Statement *stmt = parseStatement();
        if (stmt) {
          ifStmt->elseBlock.append(stmt);
        }
      }
      consume(TokenType::RightBrace, "Expected '}' after else body");
    } else {
      Statement *stmt = parseStatement();
      if (stmt) {
        ifStmt->elseBlock.append(stmt);
      }
    }
  }

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
  if (match(TokenType::Keyword_StaticCast)) {
    return parseStaticCastExpression();
  }

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

Expression *CppParser::parseStaticCastExpression() {
  if (match(TokenType::Less)) {
    int templateDepth = 1;
    while (!isAtEnd() && templateDepth > 0) {
      if (match(TokenType::Less)) {
        templateDepth++;
      } else if (match(TokenType::Greater)) {
        templateDepth--;
      } else {
        advance();
      }
    }
  }

  Expression *innerExpr = nullptr;

  if (match(TokenType::LeftParen)) {
    int parenDepth = 1;
    while (!isAtEnd() && parenDepth > 0) {
      if (check(TokenType::LeftParen)) {
        parenDepth++;
        advance();
        continue;
      }
      if (check(TokenType::RightParen)) {
        parenDepth--;
        advance();
        continue;
      }
      if (!innerExpr && check(TokenType::Keyword_New)) {
        innerExpr = parsePrimary();
        continue;
      }
      advance();
    }
  }

  if (innerExpr) {
    return innerExpr;
  }

  return new IdentifierExpr("static_cast");
}

void CppParser::skipEnumDeclaration() {
  match(TokenType::Keyword_Class);
  match(TokenType::Keyword_Struct);

  if (check(TokenType::Identifier)) {
    advance();
  }

  if (match(TokenType::LeftBrace)) {
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
    match(TokenType::Semicolon);
    return;
  }

  while (!check(TokenType::Semicolon) && !isAtEnd()) {
    advance();
  }
  match(TokenType::Semicolon);
}

} // namespace FSMParser
