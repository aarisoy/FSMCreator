#include "ModelBuilder.h"
#include <QDebug>

namespace FSMParser {

ModelBuilder::ModelBuilder(FSM *fsm) : m_fsm(fsm) {}

QString ModelBuilder::extractStateName(const QString &className) {
  // Remove "State" suffix if present
  if (className.endsWith("State") && className.length() > 5) {
    return className.left(className.length() - 5);
  }
  return className;
}

bool ModelBuilder::isStateClass(const ClassDecl *decl) {
  // Check inheritance
  if (decl->baseClass == "MyFSMStateBase" || decl->baseClass == "State") {
    return true;
  }
  // Fallback to name suffix
  return decl->name.endsWith("State") && decl->name != "BaseState";
}

void ModelBuilder::build(const QVector<ASTNode *> &nodes) {
  QVector<ClassDecl *> classes;
  QVector<EnumDecl *> enums;

  for (ASTNode *node : nodes) {
    if (auto *classDecl = dynamic_cast<ClassDecl *>(node)) {
      classes.append(classDecl);
    } else if (auto *enumDecl = dynamic_cast<EnumDecl *>(node)) {
      enums.append(enumDecl);
    }
  }

  // First pass: Create states from enums
  for (EnumDecl *enumDecl : enums) {
    enumDecl->accept(this);
  }

  // Second pass: Create all states from classes
  for (ClassDecl *classDecl : classes) {
    if (isStateClass(classDecl)) {
      if (m_stateMap.contains(classDecl->name)) {
        continue;
      }
      QString stateName = extractStateName(classDecl->name);
      State *state = new State(stateName, stateName, m_fsm);
      m_fsm->addState(state);
      m_stateMap.insert(classDecl->name, state);
    }
  }

  // Third pass: Extract transitions from handle() methods
  for (ClassDecl *classDecl : classes) {
    if (isStateClass(classDecl)) {
      m_currentState = m_stateMap.value(classDecl->name);
      classDecl->accept(this);
    }
  }
}

void ModelBuilder::visitClassDecl(ClassDecl *node) {
  // Visit all methods
  for (FunctionDecl *method : node->methods) {
    method->accept(this);
  }
}

void ModelBuilder::visitEnumDecl(EnumDecl *node) {
  if (node->name != "States") {
    return;
  }

  for (const QString &entry : node->enumerators) {
    if (!m_stateMap.contains(entry)) {
      State *state = new State(entry, entry, m_fsm);
      m_fsm->addState(state);
      m_stateMap.insert(entry, state);
    }
  }
}

void ModelBuilder::visitFunctionDecl(FunctionDecl *node) {
  // Debug log
  qDebug() << "ModelBuilder: Visiting function" << node->name;

  // Reconstruct signature
  QString signature = node->returnType + " " + node->name + "(";
  for (int i = 0; i < node->parameters.size(); ++i) {
    if (i > 0)
      signature += ", ";
    signature += node->parameters[i].type;
    if (!node->parameters[i].name.isEmpty()) {
      signature += " " + node->parameters[i].name;
    }
  }
  signature += ")";
  if (node->isOverride)
    signature += " override";

  // Only proceed if we are currently visiting a State class
  if (!m_currentState)
    return;

  if (node->name == "handle") {
    // Visit function body to find transitions (existing logic)
    for (Statement *stmt : node->body) {
      if (stmt) {
        stmt->accept(this);
      }
    }
  } else if (node->name == "enter") {
    // Assuming body contains the action code
    // meaningful code extraction is hard without more AST support for printing,
    // but we can at least mark it or store the signature?
    // For now, let's store the signature as the action, or leave empty to
    // indicate existence.
    m_currentState->setEntryAction(signature);
  } else if (node->name == "exit") {
    m_currentState->setExitAction(signature);
  } else {
    // Custom Function logic
    m_currentState->addFunction(signature);
  }
}

void ModelBuilder::visitIfStatement(IfStatement *node) {
  // Reset context
  m_currentEventName = "";
  m_currentTargetState = "";

  // Visit condition to extract event name
  if (node->condition) {
    node->condition->accept(this);
  }

  QString eventName =
      m_currentEventName.isEmpty() ? "Event" : m_currentEventName;

  // Visit then block to find target state
  for (Statement *stmt : node->thenBlock) {
    m_currentTargetState = ""; // Reset for each statement
    if (stmt) {
      stmt->accept(this);

      // If we found a target state, create transition
      if (!m_currentTargetState.isEmpty() && m_currentState) {
        State *targetState = m_stateMap.value(m_currentTargetState);

        if (!targetState) {
          // Lazy creation of target state
          QString stateName = extractStateName(m_currentTargetState);
          targetState = new State(stateName, stateName, m_fsm);
          m_fsm->addState(targetState);
          m_stateMap.insert(m_currentTargetState, targetState);
          qDebug() << "ModelBuilder: Lazily created state" << stateName
                   << "from" << m_currentTargetState;
        }

        if (targetState) {
          // Check if transition already exists
          bool exists = false;
          for (Transition *t : m_currentState->transitions()) {
            if (t->targetState() == targetState && t->event() == eventName) {
              exists = true;
              break;
            }
          }

          if (!exists) {
            Transition *transition =
                new Transition(m_currentState, targetState);
            transition->setEvent(eventName);
            m_currentState->addTransition(transition);
            m_fsm->addTransition(transition); // Add to FSM's transition list
            qDebug() << "ModelBuilder: Added transition" << eventName << "->"
                     << targetState->name();
          }
        }
      }
    }
  }
}

void ModelBuilder::visitReturnStatement(ReturnStatement *node) {
  if (node->value) {
    node->value->accept(this);
  }
}

void ModelBuilder::visitIdentifierExpr(IdentifierExpr *node) {
  // Could be part of event or state name
  // Context determines meaning
}

void ModelBuilder::visitStringLiteralExpr(StringLiteralExpr *node) {
  // This is likely an event name from: event.type == "EventName"
  // Remove quotes
  QString value = node->value;
  if (value.startsWith('"') && value.endsWith('"')) {
    value = value.mid(1, value.length() - 2);
  }
  m_currentEventName = value;
}

void ModelBuilder::visitBinaryExpr(BinaryExpr *node) {
  // Visit both sides to extract event names
  if (node->left) {
    node->left->accept(this);
  }
  if (node->right) {
    node->right->accept(this);
  }
}

void ModelBuilder::visitMemberAccessExpr(MemberAccessExpr *node) {
  // Could be event.type - visit to extract context
  if (node->object) {
    node->object->accept(this);
  }
}

void ModelBuilder::visitNewExpr(NewExpr *node) {
  // This is the target state: return new State2State();
  m_currentTargetState = node->typeName;
}

} // namespace FSMParser
