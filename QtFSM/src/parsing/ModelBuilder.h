#pragma once

#include "../model/FSM.h"
#include "../model/State.h"
#include "../model/Transition.h"
#include "AST.h"
#include <QMap>
#include <QString>

namespace FSMParser {

/**
 * @brief The ModelBuilder class traverses the AST to construct the FSM Model.
 *
 * It is an AST Visitor that interprets the parsed C++ structures (Classes,
 * Functions) and translates them into @ref State and @ref Transition objects in
 * the @ref FSM.
 *
 * @ingroup Parsing
 */
class ModelBuilder : public ASTVisitor {
public:
  /**
   * @brief Constructs a new ModelBuilder.
   * @param fsm The target FSM to populate.
   */
  explicit ModelBuilder(FSM *fsm);

  /**
   * @brief Builds the model from a list of class declarations.
   * @param nodes The vector of parsed AST nodes.
   */
  void build(const QVector<ASTNode *> &nodes);

  // Visitor methods
  void visitClassDecl(ClassDecl *node) override;
  void visitEnumDecl(EnumDecl *node) override;
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
