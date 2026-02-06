#pragma once

#include <QString>
#include <QVector>
#include <memory>

namespace FSMParser {

// Forward declarations
class ASTVisitor;

// Base AST Node
class ASTNode {
public:
  virtual ~ASTNode() = default;
  virtual void accept(ASTVisitor *visitor) = 0;
};

// Expression nodes
class Expression : public ASTNode {
public:
  virtual ~Expression() = default;
};

class IdentifierExpr : public Expression {
public:
  QString name;

  explicit IdentifierExpr(const QString &n) : name(n) {}
  void accept(ASTVisitor *visitor) override;
};

class StringLiteralExpr : public Expression {
public:
  QString value;

  explicit StringLiteralExpr(const QString &v) : value(v) {}
  void accept(ASTVisitor *visitor) override;
};

class BinaryExpr : public Expression {
public:
  std::unique_ptr<Expression> left;
  QString op;
  std::unique_ptr<Expression> right;

  BinaryExpr(Expression *l, const QString &o, Expression *r)
      : left(l), op(o), right(r) {}
  void accept(ASTVisitor *visitor) override;
};

class MemberAccessExpr : public Expression {
public:
  std::unique_ptr<Expression> object;
  QString member;

  MemberAccessExpr(Expression *obj, const QString &mem)
      : object(obj), member(mem) {}
  void accept(ASTVisitor *visitor) override;
};

class NewExpr : public Expression {
public:
  QString typeName;

  explicit NewExpr(const QString &type) : typeName(type) {}
  void accept(ASTVisitor *visitor) override;
};

// Statement nodes
class Statement : public ASTNode {
public:
  virtual ~Statement() = default;
};

class ReturnStatement : public Statement {
public:
  std::unique_ptr<Expression> value;

  explicit ReturnStatement(Expression *v = nullptr) : value(v) {}
  void accept(ASTVisitor *visitor) override;
};

class IfStatement : public Statement {
public:
  std::unique_ptr<Expression> condition;
  QVector<Statement *> thenBlock;
  QVector<Statement *> elseBlock;

  IfStatement(Expression *cond) : condition(cond) {}
  ~IfStatement() {
    qDeleteAll(thenBlock);
    qDeleteAll(elseBlock);
  }
  void accept(ASTVisitor *visitor) override;
};

// Declaration nodes
class Parameter {
public:
  QString type;
  QString name;

  Parameter(const QString &t = "", const QString &n = "") : type(t), name(n) {}
};

class FunctionDecl : public ASTNode {
public:
  QString returnType;
  QString name;
  QVector<Parameter> parameters;
  QVector<Statement *> body;
  bool isVirtual = false;
  bool isOverride = false;

  FunctionDecl(const QString &ret, const QString &n)
      : returnType(ret), name(n) {}
  ~FunctionDecl() { qDeleteAll(body); }
  void accept(ASTVisitor *visitor) override;
};

class ClassDecl : public ASTNode {
public:
  QString name;
  QString baseClass;
  QVector<FunctionDecl *> methods;

  ClassDecl(const QString &n) : name(n) {}
  ~ClassDecl() { qDeleteAll(methods); }
  void accept(ASTVisitor *visitor) override;
};

class EnumDecl : public ASTNode {
public:
  QString name;
  QVector<QString> enumerators;
  bool isEnumClass = false;

  EnumDecl(const QString &n, bool isClass) : name(n), isEnumClass(isClass) {}
  void accept(ASTVisitor *visitor) override;
};

// Visitor interface
class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;

  virtual void visitClassDecl(ClassDecl *node) = 0;
  virtual void visitEnumDecl(EnumDecl *node) = 0;
  virtual void visitFunctionDecl(FunctionDecl *node) = 0;
  virtual void visitIfStatement(IfStatement *node) = 0;
  virtual void visitReturnStatement(ReturnStatement *node) = 0;
  virtual void visitIdentifierExpr(IdentifierExpr *node) = 0;
  virtual void visitStringLiteralExpr(StringLiteralExpr *node) = 0;
  virtual void visitBinaryExpr(BinaryExpr *node) = 0;
  virtual void visitMemberAccessExpr(MemberAccessExpr *node) = 0;
  virtual void visitNewExpr(NewExpr *node) = 0;
};

} // namespace FSMParser
