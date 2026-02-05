#include "Token.h"

namespace FSMParser {

QString Token::typeName() const {
  switch (type) {
  case TokenType::Keyword_Class:
    return "class";
  case TokenType::Keyword_Struct:
    return "struct";
  case TokenType::Keyword_Enum:
    return "enum";
  case TokenType::Keyword_Public:
    return "public";
  case TokenType::Keyword_Private:
    return "private";
  case TokenType::Keyword_Protected:
    return "protected";
  case TokenType::Keyword_Virtual:
    return "virtual";
  case TokenType::Keyword_Override:
    return "override";
  case TokenType::Keyword_If:
    return "if";
  case TokenType::Keyword_Else:
    return "else";
  case TokenType::Keyword_Return:
    return "return";
  case TokenType::Keyword_New:
    return "new";
  case TokenType::Keyword_This:
    return "this";
  case TokenType::Keyword_Const:
    return "const";
  case TokenType::Keyword_Void:
    return "void";
  case TokenType::Keyword_Auto:
    return "auto";
  case TokenType::Keyword_StaticCast:
    return "static_cast";
  case TokenType::Identifier:
    return "identifier";
  case TokenType::StringLiteral:
    return "string";
  case TokenType::NumberLiteral:
    return "number";
  case TokenType::LeftBrace:
    return "{";
  case TokenType::RightBrace:
    return "}";
  case TokenType::LeftParen:
    return "(";
  case TokenType::RightParen:
    return ")";
  case TokenType::Semicolon:
    return ";";
  case TokenType::Colon:
    return ":";
  case TokenType::DoubleColon:
    return "::";
  case TokenType::Comma:
    return ",";
  case TokenType::Dot:
    return ".";
  case TokenType::Arrow:
    return "->";
  case TokenType::Star:
    return "*";
  case TokenType::Ampersand:
    return "&";
  case TokenType::Equal:
    return "=";
  case TokenType::EqualEqual:
    return "==";
  case TokenType::Exclaim:
    return "!";
  case TokenType::ExclaimEqual:
    return "!=";
  case TokenType::Tilde:
    return "~";
  case TokenType::EndOfFile:
    return "EOF";
  default:
    return "unknown";
  }
}

} // namespace FSMParser
