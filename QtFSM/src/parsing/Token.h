#pragma once

#include <QString>

namespace FSMParser {

enum class TokenType {
  // Keywords
  Keyword_Class,
  Keyword_Struct,
  Keyword_Public,
  Keyword_Private,
  Keyword_Protected,
  Keyword_Virtual,
  Keyword_Override,
  Keyword_If,
  Keyword_Else,
  Keyword_Return,
  Keyword_New,
  Keyword_This,
  Keyword_Const,
  Keyword_Void,
  Keyword_Enum,
  Keyword_Auto,
  Keyword_Final,

  // Identifiers and Literals
  Identifier,
  StringLiteral,
  NumberLiteral,

  // Operators and Punctuation
  LeftBrace,    // {
  RightBrace,   // }
  LeftParen,    // (
  RightParen,   // )
  Semicolon,    // ;
  Colon,        // :
  DoubleColon,  // ::
  Comma,        // ,
  Dot,          // .
  Arrow,        // ->
  Star,         // *
  Ampersand,    // &
  Equal,        // =
  EqualEqual,   // ==
  Exclaim,      // !
  Tilde,        // ~
  ExclaimEqual, // !=
  Less,         // <
  Greater,      // >
  LessEqual,    // <=
  GreaterEqual, // >=
  Plus,         // +
  Minus,        // -

  // Special
  Comment,
  Whitespace,
  EndOfFile,
  Unknown
};

struct Token {
  TokenType type;
  QString value;
  int line;
  int column;

  Token(TokenType t = TokenType::Unknown, const QString &v = "", int l = 0,
        int c = 0)
      : type(t), value(v), line(l), column(c) {}

  bool isKeyword() const {
    return type >= TokenType::Keyword_Class && type <= TokenType::Keyword_Final;
  }

  bool isOperator() const {
    return type >= TokenType::LeftBrace && type <= TokenType::Minus;
  }

  QString typeName() const;
};

} // namespace FSMParser
