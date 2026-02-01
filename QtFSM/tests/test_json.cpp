#include "../src/model/FSM.h"
#include "../src/model/State.h"
#include "../src/model/Transition.h"
#include "../src/parsing/CodeParser.h"
#include "../src/serialization/JSONSerializer.h"
#include <QDebug>
#include <QFile>
#include <QString>


int main() {
  // Read the input C++ file
  QFile inputFile("tests/test_input.cpp");
  if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "❌ Failed to open test_input.cpp";
    return 1;
  }

  QString code = inputFile.readAll();
  inputFile.close();

  // Parse the C++ code to FSM
  qDebug() << "=== PARSING C++ CODE ===";
  CodeParser parser;
  FSM *fsm = parser.parse(code);

  if (!fsm) {
    qDebug() << "❌ Failed to parse FSM";
    return 1;
  }

  fsm->setName("MyFSM");

  qDebug() << "✅ Parsed FSM with" << fsm->states().size() << "states and"
           << fsm->transitions().size() << "transitions";

  // Save to JSON
  qDebug() << "\n=== SAVING TO JSON ===";
  JSONSerializer serializer;
  if (!serializer.save(fsm, "test_output.json")) {
    qDebug() << "❌ Failed to save JSON";
    delete fsm;
    return 1;
  }

  qDebug() << "✅ Saved to test_output.json";

  // Load from JSON
  qDebug() << "\n=== LOADING FROM JSON ===";
  FSM *loadedFsm = serializer.load("test_output.json");

  if (!loadedFsm) {
    qDebug() << "❌ Failed to load JSON";
    delete fsm;
    return 1;
  }

  qDebug() << "✅ Loaded FSM:" << loadedFsm->name();
  qDebug() << "   States:" << loadedFsm->states().size();
  qDebug() << "   Transitions:" << loadedFsm->transitions().size();

  // Verify the data matches
  qDebug() << "\n=== VERIFICATION ===";
  bool success = true;

  if (fsm->states().size() != loadedFsm->states().size()) {
    qDebug() << "❌ State count mismatch!";
    success = false;
  }

  if (fsm->transitions().size() != loadedFsm->transitions().size()) {
    qDebug() << "❌ Transition count mismatch!";
    success = false;
  }

  // Display loaded FSM structure
  qDebug() << "\nLoaded FSM Structure:";
  for (State *state : loadedFsm->states()) {
    qDebug() << "State:" << state->name();
    for (Transition *t : loadedFsm->transitions()) {
      if (t->sourceState() == state) {
        qDebug() << "  →" << t->targetState()->name() << "[" << t->event()
                 << "]";
      }
    }
  }

  if (success) {
    qDebug() << "\n✅ JSON SERIALIZATION TEST PASSED!";
  } else {
    qDebug() << "\n❌ JSON SERIALIZATION TEST FAILED!";
  }

  delete fsm;
  delete loadedFsm;

  return success ? 0 : 1;
}
