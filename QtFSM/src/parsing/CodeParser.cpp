#include "CodeParser.h"
#include "../model/FSM.h"
#include "../model/State.h"
#include "../model/Transition.h"

#include <QMap>
#include <QPointF>
#include <QRegularExpression>
#include <QVector>

CodeParser::CodeParser() {}

QString CodeParser::lastError() const { return m_lastError; }

FSM *CodeParser::parse(const QString &code, QObject *parent) {
  if (code.isEmpty()) {
    m_lastError = "Empty code";
    return nullptr;
  }

  FSM *fsm = new FSM(parent);

  // Optional: parse FSM name from header comment
  QRegularExpression nameRe(
      R"(//\s*Auto-generated FSM Config\s*-\s*(.+))");
  QRegularExpressionMatch nameMatch = nameRe.match(code);
  if (nameMatch.hasMatch()) {
    fsm->setName(nameMatch.captured(1).trimmed());
  }

  // Parse initial state ID
  QRegularExpression initialRe(
      R"CFG(cfg\.initial\s*=\s*"([^"]*)"\s*;)CFG");
  QRegularExpressionMatch initialMatch = initialRe.match(code);
  if (!initialMatch.hasMatch()) {
    m_lastError = "Missing cfg.initial assignment";
    delete fsm;
    return nullptr;
  }
  QString initialId = initialMatch.captured(1);

  struct TransitionSpec {
    QString sourceId;
    QString event;
    QString targetId;
    QString guard;
    QString action;
  };
  QVector<TransitionSpec> transitions;

  // Parse state configs (strict format)
  QRegularExpression stateRe(
      R"CFG(cfg\.states\["([^"]*)"\]\s*=\s*StateConfig\{\s*"([^"]*)"\s*,\s*([-\d\.]+)\s*,\s*([-\d\.]+)\s*,\s*(true|false)\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"\s*,\s*\{(.*?)\}\s*\};)CFG",
      QRegularExpression::DotMatchesEverythingOption);

  QRegularExpressionMatchIterator stateIt = stateRe.globalMatch(code);
  QMap<QString, State *> stateMap;
  bool anyState = false;

  while (stateIt.hasNext()) {
    anyState = true;
    QRegularExpressionMatch match = stateIt.next();
    QString stateId = match.captured(1);
    QString stateName = match.captured(2);
    double posX = match.captured(3).toDouble();
    double posY = match.captured(4).toDouble();
    bool isFinal = (match.captured(5) == "true");
    QString entry = match.captured(6);
    QString exit = match.captured(7);
    QString transitionsBlock = match.captured(8);

    State *state = new State(stateId, stateName, fsm);
    state->setPosition(QPointF(posX, posY));
    state->setFinal(isFinal);
    state->setEntryAction(entry);
    state->setExitAction(exit);
    fsm->addState(state);
    stateMap.insert(stateId, state);

    // Parse transitions inside this state
    QRegularExpression transRe(
        R"CFG(\{\s*"([^"]*)"\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"\s*\})CFG");
    QRegularExpressionMatchIterator transIt =
        transRe.globalMatch(transitionsBlock);
    while (transIt.hasNext()) {
      QRegularExpressionMatch tmatch = transIt.next();
      TransitionSpec spec;
      spec.sourceId = stateId;
      spec.event = tmatch.captured(1);
      spec.targetId = tmatch.captured(2);
      spec.guard = tmatch.captured(3);
      spec.action = tmatch.captured(4);
      transitions.append(spec);
    }
  }

  if (!anyState) {
    m_lastError = "No StateConfig entries found";
    delete fsm;
    return nullptr;
  }

  // Build transitions
  for (const TransitionSpec &spec : transitions) {
    State *source = stateMap.value(spec.sourceId);
    State *target = stateMap.value(spec.targetId);
    if (!source || !target) {
      m_lastError = "Transition references unknown state ID";
      delete fsm;
      return nullptr;
    }

    Transition *transition = new Transition(source, target, fsm);
    transition->setEvent(spec.event);
    transition->setGuard(spec.guard);
    transition->setAction(spec.action);
    fsm->addTransition(transition);
  }

  // Set initial state
  State *initial = stateMap.value(initialId);
  if (!initial) {
    m_lastError = "Initial state ID not found";
    delete fsm;
    return nullptr;
  }
  initial->setInitial(true);
  fsm->setInitialState(initial);

  return fsm;
}
