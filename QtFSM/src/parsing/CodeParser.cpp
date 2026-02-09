#include "CodeParser.h"
#include "../model/Event.h"
#include "../model/FSM.h"
#include "../model/State.h"
#include "../model/Transition.h"

// New parser system
#include "CppParser.h"
#include "Lexer.h"
#if defined(FSM_ENABLE_LIBCLANG)
#include "LibClangParser.h"
#endif
#include "ModelBuilder.h"

#include <QDebug>
#include <QRegularExpression>

using namespace FSMParser;

CodeParser::CodeParser() {}

QString CodeParser::lastError() const { return m_lastError; }

FSM *CodeParser::parse(const QString &code, QObject *parent) {
  if (code.isEmpty()) {
    m_lastError = "Empty code";
    return nullptr;
  }

  FSM *fsm = new FSM(parent);

  // 1. Try to find FSM name
  // Pattern: class Name : public State/FSM ...
  // For now detailed parsing is limited, so we'll look for simple patterns
  // matching the code we generate

  try {
    QVector<ClassDecl *> classes;
#if defined(FSM_ENABLE_LIBCLANG)
    const QByteArray useLibclang = qgetenv("FSM_USE_LIBCLANG");
    if (!useLibclang.isEmpty() && useLibclang != "0") {
      LibClangParser parser;
      classes = parser.parse(code);
      if (!parser.lastError().isEmpty()) {
        m_lastError = "libclang parser error: " + parser.lastError();
        qDebug() << m_lastError;
        qDeleteAll(classes);
        delete fsm;
        return nullptr;
      }
    }
#endif

    if (classes.isEmpty()) {
      // Step 1: Tokenize
      Lexer lexer(code);
      QVector<Token> tokens = lexer.tokenize();

      // Debug: Show middle tokens
      qDebug() << "=== Tokens 30-100 ===";
      for (int i = 30; i < qMin(100, tokens.size()); i++) {
        qDebug().nospace() << "[" << i << "] Line " << tokens[i].line << ": "
                           << tokens[i].typeName() << " = '" << tokens[i].value
                           << "'";
      }

      // Step 2: Parse to AST
      CppParser parser(tokens);
      classes = parser.parse();

      if (parser.hasError()) {
        m_lastError = "Parser error: " + parser.errorMessage();
        qDebug() << m_lastError;
        qDeleteAll(classes);
        delete fsm;
        return nullptr;
      }
    }

    // Step 3: Build FSM model from AST
    ModelBuilder builder(fsm);
    builder.build(classes);

    // Cleanup AST
    qDeleteAll(classes);

    qDebug() << "✅ New parser: Parsed" << fsm->states().size() << "states";
    for (State *state : fsm->states()) {
      qDebug() << "  State:" << state->name() << "with"
               << state->transitions().size() << "transitions";
    }

  } catch (...) {
    m_lastError = "Parser exception";
    delete fsm;
    return nullptr;
  }

  // Set first found state as initial
  if (!fsm->states().isEmpty()) {
    fsm->setInitialState(fsm->states().first());
    fsm->states().first()->setInitial(true);
  }

  return fsm;
}
