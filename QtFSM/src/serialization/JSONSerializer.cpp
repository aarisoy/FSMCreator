#include "JSONSerializer.h"
#include "../model/FSM.h"
#include "../model/State.h"
#include "../model/Transition.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

JSONSerializer::JSONSerializer(QObject *parent) : QObject(parent) {}

JSONSerializer::~JSONSerializer() {}

bool JSONSerializer::save(const FSM *fsm, const QString &filepath) {
  if (!fsm) {
    qDebug() << "JSONSerializer::save - FSM is null";
    return false;
  }

  QJsonObject root;
  root["name"] = fsm->name();

  // Serialize states
  QJsonArray statesArray;
  for (State *state : fsm->states()) {
    QJsonObject stateObj;

    // Core identifiers
    stateObj["id"] = state->id();
    stateObj["name"] = state->name();

    // State flags
    stateObj["isInitial"] = state->isInitial();
    stateObj["isFinal"] = state->isFinal();

    // Visual position
    stateObj["positionX"] = state->position().x();
    stateObj["positionY"] = state->position().y();

    // Actions
    stateObj["entryAction"] = state->entryAction();
    stateObj["exitAction"] = state->exitAction();

    // Custom functions
    QJsonArray functionsArray;
    for (const QString &func : state->customFunctions()) {
      functionsArray.append(func);
    }
    stateObj["customFunctions"] = functionsArray;

    statesArray.append(stateObj);
  }
  root["states"] = statesArray;

  // Serialize transitions
  QJsonArray transitionsArray;
  for (Transition *transition : fsm->transitions()) {
    QJsonObject transObj;

    // Core identifiers (use IDs not names for robustness)
    transObj["id"] = transition->id();
    transObj["sourceId"] = transition->sourceState()->id();
    transObj["targetId"] = transition->targetState()->id();

    // Transition properties
    transObj["event"] = transition->event();
    transObj["guard"] = transition->guard();
    transObj["action"] = transition->action();

    transitionsArray.append(transObj);
  }
  root["transitions"] = transitionsArray;

  // Write to file
  QJsonDocument doc(root);
  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly)) {
    qDebug() << "JSONSerializer::save - Could not open file for writing:"
             << filepath;
    return false;
  }

  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  qDebug() << "FSM saved to" << filepath;
  return true;
}

FSM *JSONSerializer::load(const QString &filepath) {
  QFile file(filepath);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "JSONSerializer::load - Could not open file:" << filepath;
    return nullptr;
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull() || !doc.isObject()) {
    qDebug() << "JSONSerializer::load - Invalid JSON document";
    return nullptr;
  }

  QJsonObject root = doc.object();

  FSM *fsm = new FSM();
  fsm->setName(root["name"].toString());

  // Load states
  QMap<QString, State *> stateMapById;   // Map by ID for transitions
  QMap<QString, State *> stateMapByName; // Map by name for backward compat
  QJsonArray statesArray = root["states"].toArray();
  for (const QJsonValue &value : statesArray) {
    QJsonObject stateObj = value.toObject();

    // Support both old (name-only) and new (id + name) formats
    QString stateId = stateObj.value("id").toString();
    QString stateName = stateObj["name"].toString();

    // Backward compat: if no ID, use name as ID
    if (stateId.isEmpty()) {
      stateId = stateName;
    }

    State *state = new State(stateId, stateName, fsm);

    // Load state flags
    bool isInitial = stateObj.value("isInitial").toBool(false);
    state->setInitial(isInitial);
    state->setFinal(stateObj.value("isFinal").toBool(false));

    // Load position (defaults to 0,0 if not present)
    double posX = stateObj.value("positionX").toDouble(0.0);
    double posY = stateObj.value("positionY").toDouble(0.0);
    state->setPosition(QPointF(posX, posY));

    // Load actions
    state->setEntryAction(stateObj.value("entryAction").toString());
    state->setExitAction(stateObj.value("exitAction").toString());

    // Load custom functions
    QJsonArray functionsArray = stateObj.value("customFunctions").toArray();
    for (const QJsonValue &funcValue : functionsArray) {
      state->addFunction(funcValue.toString());
    }

    stateMapById[state->id()] = state;
    stateMapByName[state->name()] = state;
    fsm->addState(state);
    if (isInitial) {
      fsm->setInitialState(state);
    }
  }

  // Load transitions
  QJsonArray transitionsArray = root["transitions"].toArray();
  for (const QJsonValue &value : transitionsArray) {
    QJsonObject transObj = value.toObject();

    // Support both new ID-based and old name-based references
    QString sourceId = transObj.value("sourceId").toString();
    QString targetId = transObj.value("targetId").toString();

    // Backward compat: fall back to name-based lookup if no IDs
    if (sourceId.isEmpty()) {
      sourceId =
          transObj["source"].toString(); // Old format used "source" with name
    }
    if (targetId.isEmpty()) {
      targetId =
          transObj["target"].toString(); // Old format used "target" with name
    }

    // Try ID-based lookup first, then name-based (for old files)
    State *source = stateMapById.value(sourceId);
    if (!source) {
      source = stateMapByName.value(sourceId);
    }

    State *target = stateMapById.value(targetId);
    if (!target) {
      target = stateMapByName.value(targetId);
    }

    if (source && target) {
      QString transitionId = transObj.value("id").toString();
      Transition *trans;

      if (transitionId.isEmpty()) {
        // Old format: no ID
        trans = new Transition(source, target, fsm);
      } else {
        // New format: has ID
        trans = new Transition(transitionId, source, target, fsm);
      }

      // Load transition properties
      trans->setEvent(transObj.value("event").toString());
      trans->setGuard(transObj.value("guard").toString());
      trans->setAction(transObj.value("action").toString());

      fsm->addTransition(trans);
    } else {
      qDebug() << "JSONSerializer::load - Could not find source or target "
                  "state for transition";
    }
  }

  qDebug() << "FSM loaded from" << filepath;
  return fsm;
}
