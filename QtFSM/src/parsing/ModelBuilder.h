#pragma once

#include "../model/FSM.h"
#include "../model/State.h"
#include "../model/Transition.h"
#include "AST.h"
#include <QMap>
#include <QString>

namespace FSMParser {

class ModelBuilder : public ASTVisitor {
public:
  explicit ModelBuilder(FSM *fsm);

  // Build model from AST
  void build(const QVector<ClassDecl *> &classes);

  // Visitor methods
  void visitClassDecl(ClassDecl *node) override;
  void visitFunctionDecl(FunctionDecl *node) override;
  void visitIfStatement(IfStatement *node) override;
  void visitReturnStatement(ReturnStatement *node) override;
  void visitIdentifierExpr(IdentifierExpr *node) override;
  void visitStringLiteralExpr(StringLiteralExpr *node) override;
  void visitBinaryExpr(BinaryExpr *node) override;
  void visitMemberAccessExpr(MemberAccessExpr *node) override;
  void visitNewExpr(NewExpr *node) override;

private:
  FSM *m_fsm;
  QMap<QString, State *> m_stateMap; // Class name → State

  // Current context during traversal
  State *m_currentState = nullptr;
  QString m_currentEventName;
  QString m_currentTargetState;

  // Helper to extract state name from class name (e.g., "State1State" →
  // "State1")
  QString extractStateName(const QString &className);

  // Check if a class is a state class
  bool isStateClass(const ClassDecl *decl);
};

} // namespace FSMParser
