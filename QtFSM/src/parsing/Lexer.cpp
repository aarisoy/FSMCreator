#include "Lexer.h"

namespace FSMParser {

Lexer::Lexer(const QString &source) : m_source(source) { initKeywords(); }

void Lexer::initKeywords() {
  m_keywords["class"] = TokenType::Keyword_Class;
  m_keywords["struct"] = TokenType::Keyword_Struct;
  m_keywords["public"] = TokenType::Keyword_Public;
  m_keywords["private"] = TokenType::Keyword_Private;
  m_keywords["protected"] = TokenType::Keyword_Protected;
  m_keywords["virtual"] = TokenType::Keyword_Virtual;
  m_keywords["override"] = TokenType::Keyword_Override;
  m_keywords["if"] = TokenType::Keyword_If;
  m_keywords["else"] = TokenType::Keyword_Else;
  m_keywords["return"] = TokenType::Keyword_Return;
  m_keywords["new"] = TokenType::Keyword_New;
  m_keywords["this"] = TokenType::Keyword_This;
  m_keywords["const"] = TokenType::Keyword_Const;
  m_keywords["void"] = TokenType::Keyword_Void;
  m_keywords["enum"] = TokenType::Keyword_Enum;
  m_keywords["auto"] = TokenType::Keyword_Auto;
  m_keywords["final"] = TokenType::Keyword_Final;
}

QChar Lexer::peek() const {
  if (isAtEnd())
    return '\0';
  return m_source[m_current];
}

QChar Lexer::peekNext() const {
  if (m_current + 1 >= m_source.length())
    return '\0';
  return m_source[m_current + 1];
}

QChar Lexer::advance() {
  if (isAtEnd())
    return '\0';
  QChar c = m_source[m_current++];
  if (c == '\n') {
    m_line++;
    m_column = 1;
  } else {
    m_column++;
  }
  return c;
}

bool Lexer::match(QChar expected) {
  if (isAtEnd())
    return false;
  if (peek() != expected)
    return false;
  advance();
  return true;
}

void Lexer::skipWhitespace() {
  while (!isAtEnd()) {
    QChar c = peek();
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
      advance();
    } else if (c == '/' && peekNext() == '/') {
      // Line comment
      while (!isAtEnd() && peek() != '\n')
        advance();
    } else if (c == '/' && peekNext() == '*') {
      // Block comment
      advance(); // /
      advance(); // *
      while (!isAtEnd()) {
        if (peek() == '*' && peekNext() == '/') {
          advance(); // *
          advance(); // /
          break;
        }
        advance();
      }
    } else {
      break;
    }
  }
}

Token Lexer::scanIdentifierOrKeyword() {
  int start = m_current;
  while (!isAtEnd() && (peek().isLetterOrNumber() || peek() == '_')) {
    advance();
  }

  QString text = m_source.mid(start, m_current - start);
  if (m_keywords.contains(text)) {
    return makeToken(m_keywords[text], text);
  }
  return makeToken(TokenType::Identifier, text);
}

Token Lexer::scanStringLiteral() {
  int start = m_current;
  advance(); // Opening "

  while (!isAtEnd() && peek() != '"') {
    if (peek() == '\\') {
      advance(); // escape
      if (!isAtEnd())
        advance(); // escaped char
    } else {
      advance();
    }
  }

  if (isAtEnd()) {
    m_errorMessage = "Unterminated string literal";
    return makeToken(TokenType::Unknown);
  }

  advance(); // Closing "
  QString text = m_source.mid(start, m_current - start);
  return makeToken(TokenType::StringLiteral, text);
}

Token Lexer::scanNumber() {
  int start = m_current;
  while (!isAtEnd() && peek().isDigit()) {
    advance();
  }

  QString text = m_source.mid(start, m_current - start);
  return makeToken(TokenType::NumberLiteral, text);
}

Token Lexer::makeToken(TokenType type, const QString &value) {
  return Token(type, value, m_line, m_column);
}

Token Lexer::nextToken() {
  skipWhitespace();

  if (isAtEnd()) {
    return makeToken(TokenType::EndOfFile);
  }

  QChar c = peek();

  // Identifiers and keywords
  if (c.isLetter() || c == '_') {
    return scanIdentifierOrKeyword();
  }

  // Numbers
  if (c.isDigit()) {
    return scanNumber();
  }

  // String literals
  if (c == '"') {
    return scanStringLiteral();
  }

  // Two-character operators
  advance();
  switch (c.toLatin1()) {
  case '{':
    return makeToken(TokenType::LeftBrace, "{");
  case '}':
    return makeToken(TokenType::RightBrace, "}");
  case '(':
    return makeToken(TokenType::LeftParen, "(");
  case ')':
    return makeToken(TokenType::RightParen, ")");
  case ';':
    return makeToken(TokenType::Semicolon, ";");
  case ',':
    return makeToken(TokenType::Comma, ",");
  case '.':
    return makeToken(TokenType::Dot, ".");
  case '*':
    return makeToken(TokenType::Star, "*");
  case '&':
    return makeToken(TokenType::Ampersand, "&");
  case '+':
    return makeToken(TokenType::Plus, "+");
  case '~':
    return makeToken(TokenType::Tilde, "~");
  case '<':
    if (match('='))
      return makeToken(TokenType::LessEqual, "<=");
    return makeToken(TokenType::Less, "<");
  case '>':
    if (match('='))
      return makeToken(TokenType::GreaterEqual, ">=");
    return makeToken(TokenType::Greater, ">");
  case '=':
    if (match('='))
      return makeToken(TokenType::EqualEqual, "==");
    return makeToken(TokenType::Equal, "=");
  case '!':
    if (match('='))
      return makeToken(TokenType::ExclaimEqual, "!=");
    return makeToken(TokenType::Exclaim, "!");
  case ':':
    if (match(':'))
      return makeToken(TokenType::DoubleColon, "::");
    return makeToken(TokenType::Colon, ":");
  case '-':
    if (match('>'))
      return makeToken(TokenType::Arrow, "->");
    return makeToken(TokenType::Minus, "-");
  default:
    return makeToken(TokenType::Unknown, QString(c));
  }
}

Token Lexer::peekToken() {
  int savedCurrent = m_current;
  int savedLine = m_line;
  int savedColumn = m_column;

  Token token = nextToken();

  m_current = savedCurrent;
  m_line = savedLine;
  m_column = savedColumn;

  return token;
}

QVector<Token> Lexer::tokenize() {
  QVector<Token> tokens;
  while (!isAtEnd()) {
    Token token = nextToken();
    tokens.append(token);
    if (token.type == TokenType::EndOfFile)
      break;
  }
  return tokens;
}

} // namespace FSMParser
