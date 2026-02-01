#include "../src/model/FSM.h"
#include "../src/model/State.h"
#include "../src/model/Transition.h"
#include "../src/parsing/CodeParser.h"
#include <QSet>
#include <QString>
#include <QVector>
#include <gtest/gtest.h>


// GTest for Full Parser Functionality
TEST(CodeParserTest, ParsesCompleteStateMachine) {
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

  // Parse the code
  CodeParser parser;
  FSM *fsm = parser.parse(testCode);

  ASSERT_NE(fsm, nullptr) << "Parser should successfully parse the code";

  // Verify we have 3 states
  ASSERT_EQ(fsm->states().size(), 3) << "Should parse 3 states";

  // Verify state names
  QSet<QString> stateNames;
  for (State *state : fsm->states()) {
    stateNames.insert(state->name());
  }

  EXPECT_TRUE(stateNames.contains("State1")) << "Should have State1";
  EXPECT_TRUE(stateNames.contains("State2")) << "Should have State2";
  EXPECT_TRUE(stateNames.contains("Error")) << "Should have Error state";

  // Expected transitions
  struct ExpectedTransition {
    QString source;
    QString target;
    QString event;
  };

  QVector<ExpectedTransition> expected = {{"State1", "State2", "ButtonClick"},
                                          {"State1", "Error", "ErrorOccurred"},
                                          {"State2", "State1", "BackPressed"}};

  // Verify all expected transitions exist
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
      if (found)
        break;
    }

    EXPECT_TRUE(found) << "Should have transition: " << exp.source.toStdString()
                       << " -> " << exp.target.toStdString() << " ["
                       << exp.event.toStdString() << "]";
  }

  delete fsm;
}
