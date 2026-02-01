#include "src/model/FSM.h"
#include "src/model/State.h"
#include "src/model/Transition.h"
#include "src/parsing/CodeParser.h"
#include <QCoreApplication>
#include <QDebug>


int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  // Test code sample - similar to what the code generator produces
  QString testCode = R"(
#include <memory>
#include <string>

// Event structure
struct Event {
    std::string type;
};

class MyFSMStateBase {
public:
    virtual ~MyFSMStateBase() = default;
    virtual MyFSMStateBase* handle(MyFSMContext* context, const Event& event) = 0;
    virtual std::string getName() const = 0;
};

// State1 State
class State1State : public MyFSMStateBase {
public:
    MyFSMStateBase* handle(MyFSMContext* context, const Event& event) override {
        if (event.type == "ButtonClick") {
            return new State2State();
        }
        if (event.type == "ErrorOccurred") {
            return new ErrorState();
        }
        return nullptr; // Stay in current state
    }
    std::string getName() const override { return "State1"; }
};

// State2 State  
class State2State : public MyFSMStateBase {
public:
    MyFSMStateBase* handle(MyFSMContext* context, const Event& event) override {
        if (event.type == "BackPressed") {
            return new State1State();
        }
        return nullptr;
    }
    std::string getName() const override { return "State2"; }
};

// ErrorState
class ErrorState : public MyFSMStateBase {
public:
    MyFSMStateBase* handle(MyFSMContext* context, const Event& event) override {
        return nullptr; // Dead end state
    }
    std::string getName() const override { return "Error"; }
};
)";

  qDebug() << "========================================";
  qDebug() << "Testing Code Parser";
  qDebug() << "========================================\n";

  // Parse the code
  CodeParser parser;
  FSM *fsm = parser.parse(testCode);

  if (!fsm) {
    qDebug() << "❌ FAILED: Parser returned nullptr";
    qDebug() << "Error:" << parser.lastError();
    return 1;
  }

  qDebug() << "✅ Parser succeeded!";
  qDebug() << "\n--- States Found ---";
  for (State *state : fsm->states()) {
    qDebug() << "  •" << state->name();
  }

  qDebug() << "\n--- Transitions Found ---";
  bool allPassed = true;

  // Expected transitions
  struct ExpectedTransition {
    QString source;
    QString target;
    QString event;
  };

  QVector<ExpectedTransition> expected = {{"State1", "State2", "ButtonClick"},
                                          {"State1", "Error", "ErrorOccurred"},
                                          {"State2", "State1", "BackPressed"}};

  // Check all states
  for (State *state : fsm->states()) {
    qDebug() << "\n  From" << state->name() << ":";
    for (Transition *t : state->transitions()) {
      QString source = t->sourceState()->name();
      QString target = t->targetState()->name();
      QString event = t->event();

      qDebug() << "    ->" << target << "[" << event << "]";

      // Verify this matches expected
      bool found = false;
      for (const auto &exp : expected) {
        if (exp.source == source && exp.target == target &&
            exp.event == event) {
          found = true;
          break;
        }
      }

      if (!found) {
        qDebug() << "      ❌ UNEXPECTED transition!";
        allPassed = false;
      } else {
        qDebug() << "      ✅ Correct";
      }
    }

    if (state->transitions().isEmpty()) {
      qDebug() << "    (no transitions)";
    }
  }

  // Check if we found all expected
  qDebug() << "\n--- Verification ---";
  for (const auto &exp : expected) {
    bool found = false;
    for (State *state : fsm->states()) {
      if (state->name() != exp.source)
        continue;
      for (Transition *t : state->transitions()) {
        if (t->targetState()->name() == exp.target && t->event() == exp.event) {
          found = true;
          break;
        }
      }
    }

    if (found) {
      qDebug() << "✅ Found:" << exp.source << "->" << exp.target << "["
               << exp.event << "]";
    } else {
      qDebug() << "❌ MISSING:" << exp.source << "->" << exp.target << "["
               << exp.event << "]";
      allPassed = false;
    }
  }

  qDebug() << "\n========================================";
  if (allPassed) {
    qDebug() << "✅ ALL TESTS PASSED!";
    qDebug() << "========================================";
    delete fsm;
    return 0;
  } else {
    qDebug() << "❌ SOME TESTS FAILED";
    qDebug() << "========================================";
    delete fsm;
    return 1;
  }
}
