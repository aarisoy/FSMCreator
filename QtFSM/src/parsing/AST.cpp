#include "AST.h"

namespace FSMParser {

// Visitor accept implementations
void IdentifierExpr::accept(ASTVisitor *visitor) {
  visitor->visitIdentifierExpr(this);
}

void StringLiteralExpr::accept(ASTVisitor *visitor) {
  visitor->visitStringLiteralExpr(this);
}

void BinaryExpr::accept(ASTVisitor *visitor) { visitor->visitBinaryExpr(this); }

void MemberAccessExpr::accept(ASTVisitor *visitor) {
  visitor->visitMemberAccessExpr(this);
}

void NewExpr::accept(ASTVisitor *visitor) { visitor->visitNewExpr(this); }

void ReturnStatement::accept(ASTVisitor *visitor) {
  visitor->visitReturnStatement(this);
}

void IfStatement::accept(ASTVisitor *visitor) {
  visitor->visitIfStatement(this);
}

void FunctionDecl::accept(ASTVisitor *visitor) {
  visitor->visitFunctionDecl(this);
}

void ClassDecl::accept(ASTVisitor *visitor) { visitor->visitClassDecl(this); }

void EnumDecl::accept(ASTVisitor *visitor) { visitor->visitEnumDecl(this); }

} // namespace FSMParser
