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
    stateObj["name"] = state->name();
    stateObj["isInitial"] = state->isInitial();
    statesArray.append(stateObj);
  }
  root["states"] = statesArray;

  // Serialize transitions
  QJsonArray transitionsArray;
  for (Transition *transition : fsm->transitions()) {
    QJsonObject transObj;
    transObj["source"] = transition->sourceState()->name();
    transObj["target"] = transition->targetState()->name();
    transObj["event"] = transition->event();
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
  QMap<QString, State *> stateMap;
  QJsonArray statesArray = root["states"].toArray();
  for (const QJsonValue &value : statesArray) {
    QJsonObject stateObj = value.toObject();

    QString stateName = stateObj["name"].toString();
    State *state =
        new State(stateName, stateName, fsm); // id and name are the same
    state->setInitial(stateObj["isInitial"].toBool());

    stateMap[state->name()] = state;
    fsm->addState(state);
  }

  // Load transitions
  QJsonArray transitionsArray = root["transitions"].toArray();
  for (const QJsonValue &value : transitionsArray) {
    QJsonObject transObj = value.toObject();

    QString sourceName = transObj["source"].toString();
    QString targetName = transObj["target"].toString();
    QString event = transObj["event"].toString();

    State *source = stateMap.value(sourceName);
    State *target = stateMap.value(targetName);

    if (source && target) {
      Transition *trans = new Transition(source, target, fsm);
      trans->setEvent(event);
      fsm->addTransition(trans);
    } else {
      qDebug() << "JSONSerializer::load - Could not find source or target "
                  "state for transition";
    }
  }

  qDebug() << "FSM loaded from" << filepath;
  return fsm;
}
