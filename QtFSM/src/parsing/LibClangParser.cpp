#include "LibClangParser.h"
#include "AST.h"

#include <QDebug>
#include <QStringList>
#include <algorithm>
#include <vector>

#include <clang-c/Index.h>

namespace {
QString toQString(CXString value) {
  QString result = QString::fromUtf8(clang_getCString(value));
  clang_disposeString(value);
  return result;
}

QString cleanTypeName(const QString &typeName) {
  QString cleaned = typeName.trimmed();
  cleaned.remove("class ");
  cleaned.remove("struct ");
  cleaned.remove("&");
  cleaned.remove("*");
  return cleaned.trimmed();
}

bool isExpressionKind(CXCursorKind kind) {
  switch (kind) {
  case CXCursor_BinaryOperator:
  case CXCursor_CallExpr:
  case CXCursor_CXXBoolLiteralExpr:
  case CXCursor_DeclRefExpr:
  case CXCursor_IntegerLiteral:
  case CXCursor_MemberRefExpr:
  case CXCursor_CXXNewExpr:
  case CXCursor_StringLiteral:
  case CXCursor_UnexposedExpr:
    return true;
  default:
    return false;
  }
}

std::unique_ptr<FSMParser::Expression> parseExpression(CXCursor cursor);

QString buildQualifiedName(CXCursor cursor) {
  if (clang_Cursor_isNull(cursor)) {
    return {};
  }

  std::vector<QString> parts;
  CXCursor current = cursor;
  while (!clang_Cursor_isNull(current)) {
    const QString part = toQString(clang_getCursorSpelling(current));
    if (!part.isEmpty()) {
      parts.push_back(part);
    }
    const CXCursor parent = clang_getCursorSemanticParent(current);
    if (clang_Cursor_isNull(parent) ||
        clang_getCursorKind(parent) == CXCursor_TranslationUnit) {
      break;
    }
    current = parent;
  }

  if (parts.empty()) {
    return {};
  }

  std::reverse(parts.begin(), parts.end());
  return QStringList::fromVector(QVector<QString>(parts.begin(), parts.end()))
      .join("::");
}

QString extractQualifiedName(CXCursor cursor) {
  const CXCursor referenced = clang_getCursorReferenced(cursor);
  const QString qualified = buildQualifiedName(referenced);
  if (!qualified.isEmpty()) {
    return qualified;
  }
  return toQString(clang_getCursorSpelling(cursor));
}

std::unique_ptr<FSMParser::Expression> parseUnexposedExpr(CXCursor cursor) {
  std::unique_ptr<FSMParser::Expression> result;
  clang_visitChildren(
      cursor,
      [](CXCursor child, CXCursor, CXClientData clientData) {
        auto *expression =
            static_cast<std::unique_ptr<FSMParser::Expression> *>(clientData);
        if (*expression) {
          return CXChildVisit_Break;
        }
        if (isExpressionKind(clang_getCursorKind(child))) {
          *expression = parseExpression(child);
          return CXChildVisit_Break;
        }
        return CXChildVisit_Recurse;
      },
      &result);
  return result;
}

std::unique_ptr<FSMParser::Expression> parseBinaryExpr(CXCursor cursor) {
  std::unique_ptr<FSMParser::Expression> left;
  std::unique_ptr<FSMParser::Expression> right;
  struct ExprPair {
    std::unique_ptr<FSMParser::Expression> *left;
    std::unique_ptr<FSMParser::Expression> *right;
  };
  ExprPair pair{&left, &right};
  clang_visitChildren(
      cursor,
      [](CXCursor child, CXCursor, CXClientData clientData) {
        auto *pair = static_cast<ExprPair *>(clientData);
        if (!*pair->left) {
          *pair->left = parseExpression(child);
          return CXChildVisit_Continue;
        }
        if (!*pair->right) {
          *pair->right = parseExpression(child);
          return CXChildVisit_Continue;
        }
        return CXChildVisit_Break;
      },
      &pair);

  if (!left || !right) {
    return nullptr;
  }

  return std::make_unique<FSMParser::BinaryExpr>(left.release(), "",
                                                 right.release());
}

std::unique_ptr<FSMParser::Expression> parseExpression(CXCursor cursor) {
  const CXCursorKind kind = clang_getCursorKind(cursor);
  switch (kind) {
  case CXCursor_DeclRefExpr:
    return std::make_unique<FSMParser::IdentifierExpr>(
        extractQualifiedName(cursor));
  case CXCursor_CallExpr:
    return std::make_unique<FSMParser::IdentifierExpr>(
        extractQualifiedName(cursor));
  case CXCursor_CXXBoolLiteralExpr:
    return std::make_unique<FSMParser::StringLiteralExpr>(
        toQString(clang_getCursorSpelling(cursor)));
  case CXCursor_IntegerLiteral:
    return std::make_unique<FSMParser::StringLiteralExpr>(
        toQString(clang_getCursorSpelling(cursor)));
  case CXCursor_StringLiteral:
    return std::make_unique<FSMParser::StringLiteralExpr>(
        toQString(clang_getCursorSpelling(cursor)));
  case CXCursor_MemberRefExpr: {
    QString member = extractQualifiedName(cursor);
    std::unique_ptr<FSMParser::Expression> object;
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor, CXClientData clientData) {
          auto *target =
              static_cast<std::unique_ptr<FSMParser::Expression> *>(clientData);
          if (*target) {
            return CXChildVisit_Break;
          }
          if (isExpressionKind(clang_getCursorKind(child))) {
            *target = parseExpression(child);
            return CXChildVisit_Break;
          }
          return CXChildVisit_Continue;
        },
        &object);
    if (!object) {
      return nullptr;
    }
    return std::make_unique<FSMParser::MemberAccessExpr>(object.release(),
                                                         member);
  }
  case CXCursor_CXXNewExpr: {
    QString typeName = cleanTypeName(toQString(clang_getTypeSpelling(
        clang_getCursorType(cursor))));
    return std::make_unique<FSMParser::NewExpr>(typeName);
  }
  case CXCursor_BinaryOperator:
    return parseBinaryExpr(cursor);
  case CXCursor_UnexposedExpr:
    return parseUnexposedExpr(cursor);
  default:
    return nullptr;
  }
}

FSMParser::Statement *parseStatement(CXCursor cursor);

QVector<FSMParser::Statement *> parseStatementBlock(CXCursor cursor) {
  QVector<FSMParser::Statement *> statements;
  if (clang_getCursorKind(cursor) == CXCursor_CompoundStmt) {
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor, CXClientData clientData) {
          auto *target =
              static_cast<QVector<FSMParser::Statement *> *>(clientData);
          if (FSMParser::Statement *statement = parseStatement(child)) {
            target->push_back(statement);
          }
          return CXChildVisit_Continue;
        },
        &statements);
  } else {
    if (FSMParser::Statement *statement = parseStatement(cursor)) {
      statements.push_back(statement);
    }
  }
  return statements;
}

FSMParser::Statement *parseIfStatement(CXCursor cursor) {
  std::vector<CXCursor> children;
  clang_visitChildren(
      cursor,
      [](CXCursor child, CXCursor, CXClientData clientData) {
        auto *list = static_cast<std::vector<CXCursor> *>(clientData);
        list->push_back(child);
        return CXChildVisit_Continue;
      },
      &children);

  if (children.empty()) {
    return nullptr;
  }

  auto *ifStmt = new FSMParser::IfStatement(nullptr);
  if (isExpressionKind(clang_getCursorKind(children[0]))) {
    ifStmt->condition = parseExpression(children[0]);
  }

  if (children.size() > 1) {
    ifStmt->thenBlock = parseStatementBlock(children[1]);
  }
  if (children.size() > 2) {
    ifStmt->elseBlock = parseStatementBlock(children[2]);
  }
  return ifStmt;
}

FSMParser::Statement *parseReturnStatement(CXCursor cursor) {
  std::unique_ptr<FSMParser::Expression> value;
  clang_visitChildren(
      cursor,
      [](CXCursor child, CXCursor, CXClientData clientData) {
        auto *target =
            static_cast<std::unique_ptr<FSMParser::Expression> *>(clientData);
        if (*target) {
          return CXChildVisit_Break;
        }
        if (isExpressionKind(clang_getCursorKind(child))) {
          *target = parseExpression(child);
          return CXChildVisit_Break;
        }
        return CXChildVisit_Recurse;
      },
      &value);
  return new FSMParser::ReturnStatement(value.release());
}

FSMParser::Statement *parseStatement(CXCursor cursor) {
  const CXCursorKind kind = clang_getCursorKind(cursor);
  switch (kind) {
  case CXCursor_IfStmt:
    return parseIfStatement(cursor);
  case CXCursor_ReturnStmt:
    return parseReturnStatement(cursor);
  default:
    return nullptr;
  }
}

FSMParser::FunctionDecl *parseFunctionDecl(CXCursor cursor) {
  QString name = toQString(clang_getCursorSpelling(cursor));
  QString returnType = toQString(
      clang_getTypeSpelling(clang_getResultType(clang_getCursorType(cursor))));
  auto *functionDecl = new FSMParser::FunctionDecl(returnType, name);

  int argCount = clang_Cursor_getNumArguments(cursor);
  for (int i = 0; i < argCount; ++i) {
    CXCursor arg = clang_Cursor_getArgument(cursor, i);
    QString argName = toQString(clang_getCursorSpelling(arg));
    QString argType = toQString(clang_getTypeSpelling(clang_getCursorType(arg)));
    functionDecl->parameters.push_back({argType, argName});
  }

  functionDecl->isVirtual = clang_CXXMethod_isVirtual(cursor);

  clang_visitChildren(
      cursor,
      [](CXCursor child, CXCursor, CXClientData clientData) {
        auto *target =
            static_cast<QVector<FSMParser::Statement *> *>(clientData);
        if (clang_getCursorKind(child) == CXCursor_CompoundStmt) {
          *target = parseStatementBlock(child);
          return CXChildVisit_Break;
        }
        return CXChildVisit_Continue;
      },
      &functionDecl->body);

  return functionDecl;
}

FSMParser::ClassDecl *parseClassDecl(CXCursor cursor) {
  QString name = toQString(clang_getCursorSpelling(cursor));
  if (name.isEmpty()) {
    return nullptr;
  }
  auto *classDecl = new FSMParser::ClassDecl(name);

  clang_visitChildren(
      cursor,
      [](CXCursor child, CXCursor, CXClientData clientData) {
        auto *target = static_cast<FSMParser::ClassDecl *>(clientData);
        const CXCursorKind kind = clang_getCursorKind(child);
        if (kind == CXCursor_CXXBaseSpecifier) {
          QString baseType = cleanTypeName(
              toQString(clang_getTypeSpelling(clang_getCursorType(child))));
          if (!baseType.isEmpty()) {
            target->baseClass = baseType;
          }
        } else if (kind == CXCursor_CXXMethod) {
          target->methods.push_back(parseFunctionDecl(child));
        }
        return CXChildVisit_Continue;
      },
      classDecl);

  return classDecl;
}

} // namespace

namespace FSMParser {

LibClangParser::LibClangParser() {}

QVector<ClassDecl *> LibClangParser::parse(const QString &code) {
  QVector<ClassDecl *> classes;
  m_lastError.clear();

  if (code.trimmed().isEmpty()) {
    m_lastError = "Empty code";
    return classes;
  }

  CXIndex index = clang_createIndex(0, 0);
  const char *fileName = "input.cpp";
  QByteArray utf8 = code.toUtf8();
  CXUnsavedFile unsaved;
  unsaved.Filename = fileName;
  unsaved.Contents = utf8.constData();
  unsaved.Length = static_cast<unsigned long>(utf8.size());

  const char *args[] = {"-x", "c++", "-std=c++17"};
  CXTranslationUnit translationUnit =
      clang_parseTranslationUnit(index, fileName, args, 3, &unsaved, 1,
                                 CXTranslationUnit_None);

  if (!translationUnit) {
    m_lastError = "Failed to parse code with libclang.";
    clang_disposeIndex(index);
    return classes;
  }

  CXCursor root = clang_getTranslationUnitCursor(translationUnit);
  clang_visitChildren(
      root,
      [](CXCursor cursor, CXCursor, CXClientData clientData) {
        auto *target = static_cast<QVector<ClassDecl *> *>(clientData);
        const CXCursorKind kind = clang_getCursorKind(cursor);
        if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl) {
          if (ClassDecl *decl = parseClassDecl(cursor)) {
            target->push_back(decl);
          }
        }
        return CXChildVisit_Continue;
      },
      &classes);

  clang_disposeTranslationUnit(translationUnit);
  clang_disposeIndex(index);

  return classes;
}

QString LibClangParser::lastError() const { return m_lastError; }

} // namespace FSMParser
