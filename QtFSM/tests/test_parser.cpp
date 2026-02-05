#include "../src/model/FSM.h"
#include "../src/model/State.h"
#include "../src/model/Transition.h"
#include "../src/parsing/CodeParser.h"
#include "../src/parsing/CppParser.h"
#include "../src/parsing/Lexer.h"
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

// Test for Namespaced Types Support (std::string, ABC::func_name)
TEST(CodeParserTest, ParsesNamespacedTypes) {
  QString testCode = R"(
#include <memory>
#include <string>

class NamespaceTestState : public MyFSMStateBase {
public:
    // Test 1: std::string as return type
    std::string getName() const override {
        return "test";
    }
    
    // Test 2: Namespaced types in parameters
    void processEvent(const std::string& eventName, int count) {
        // body
    }
    
    // Test 3: Pointer to namespaced type
    std::shared_ptr<State> getNextState() {
        return nullptr;
    }
    
    // Test 4: Custom namespace
    MyNamespace::MyType getCustomType() const {
        return MyNamespace::MyType();
    }
    
    // Test 5: const qualified namespaced type
    const std::string& getReference() const {
        return myString;
    }
    
    MyFSMStateBase* handle(MyFSMContext* context, const Event& event) override {
        if (event.type == "TestEvent") {
            return new NamespaceTestState();
        }
        return nullptr;
    }
};
)";

  // Parse the code
  CodeParser parser;
  FSM *fsm = parser.parse(testCode);

  ASSERT_NE(fsm, nullptr)
      << "Parser should successfully parse namespaced types";
  ASSERT_EQ(fsm->states().size(), 1) << "Should parse 1 state";

  State *state = fsm->states()[0];
  EXPECT_EQ(state->name(), "NamespaceTest")
      << "State name should be NamespaceTest";

  // Verify transition
  ASSERT_EQ(state->transitions().size(), 1) << "Should have 1 transition";
  EXPECT_EQ(state->transitions()[0]->event(), "TestEvent");
  EXPECT_EQ(state->transitions()[0]->targetState()->name(), "NamespaceTest");

  delete fsm;
}

// Test to verify the parser correctly handles :: operator
TEST(LexerTest, TokenizesDoubleColonOperator) {
  using namespace FSMParser;

  QString source = "std::string ABC::func MyNamespace::Type";
  Lexer lexer(source);

  QVector<Token> tokens = lexer.tokenize();

  // Should have: std, ::, string, ABC, ::, func, MyNamespace, ::, Type, EOF
  ASSERT_GE(tokens.size(), 9);

  EXPECT_EQ(tokens[0].type, TokenType::Identifier);
  EXPECT_EQ(tokens[0].value, "std");

  EXPECT_EQ(tokens[1].type, TokenType::DoubleColon);
  EXPECT_EQ(tokens[1].value, "::");

  EXPECT_EQ(tokens[2].type, TokenType::Identifier);
  EXPECT_EQ(tokens[2].value, "string");

  EXPECT_EQ(tokens[3].type, TokenType::Identifier);
  EXPECT_EQ(tokens[3].value, "ABC");

  EXPECT_EQ(tokens[4].type, TokenType::DoubleColon);
  EXPECT_EQ(tokens[4].value, "::");

  EXPECT_EQ(tokens[5].type, TokenType::Identifier);
  EXPECT_EQ(tokens[5].value, "func");
}

// Test for Qualified Function Names (ABC::func_name)
TEST(CppParserTest, ParsesQualifiedFunctionNames) {
  using namespace FSMParser;

  QString testCode = R"(
class MyClass : public MyFSMStateBase {
public:
    // Simple member function
    std::string getName() const override {
        return "test";
    }
    
    MyFSMStateBase* handle(MyFSMContext* context, const Event& event) override {
        if (event.type == "Test") {
            return new MyClass();
        }
        return nullptr;
    }
};

// Out-of-class definition with qualified function name
std::string MyClass::getName() const {
    return "MyClass";
}

// Namespace function
void MyNamespace::processEvent(const Event& e) {
    // body
}
)";

  Lexer lexer(testCode);
  QVector<Token> tokens = lexer.tokenize();

  CppParser parser(tokens);
  QVector<ClassDecl *> classes = parser.parse();

  // Should successfully parse the class
  ASSERT_EQ(classes.size(), 1) << "Should parse 1 class";
  EXPECT_EQ(classes[0]->name, "MyClass");

  // The class should have methods
  EXPECT_GE(classes[0]->methods.size(), 1) << "Should have at least 1 method";

  // Clean up
  qDeleteAll(classes);
}

TEST(CodeParserTest, ParsesSpecialKeywordCases) {
  QString testCode = R"(
class MyFSMStateBase {
public:
    virtual ~MyFSMStateBase() = default;
    virtual MyFSMStateBase* handle(MyFSMContext* context, const Event& event) = 0;
    virtual std::string getName() const = 0;
};

class SpecialState : public MyFSMStateBase {
public:
    enum class Mode { Idle, Active };

    auto helper() const {
        return 42;
    }

    MyFSMStateBase* handle(MyFSMContext* context, const Event& event) override {
        if (event.type == "Start") {
            return static_cast<MyFSMStateBase*>(new ActiveState());
        } else if (event.type == "Stop") {
            return new IdleState();
        }
        return nullptr;
    }

    std::string getName() const override { return "Special"; }
};

class ActiveState : public MyFSMStateBase {
public:
    MyFSMStateBase* handle(MyFSMContext* context, const Event& event) override {
        return nullptr;
    }
    std::string getName() const override { return "Active"; }
};

class IdleState : public MyFSMStateBase {
public:
    MyFSMStateBase* handle(MyFSMContext* context, const Event& event) override {
        return nullptr;
    }
    std::string getName() const override { return "Idle"; }
};
)";

  CodeParser parser;
  FSM *fsm = parser.parse(testCode);

  ASSERT_NE(fsm, nullptr);

  State *special = fsm->stateById("Special");
  ASSERT_NE(special, nullptr);

  EXPECT_EQ(special->customFunctions().size(), 1);
  EXPECT_EQ(special->customFunctions()[0], "auto helper()")
      << "Should parse auto return type for helper()";

  ASSERT_EQ(special->transitions().size(), 2);

  QSet<QString> transitionEvents;
  QSet<QString> transitionTargets;
  for (Transition *transition : special->transitions()) {
    transitionEvents.insert(transition->event());
    transitionTargets.insert(transition->targetState()->name());
  }

  EXPECT_TRUE(transitionEvents.contains("Start"));
  EXPECT_TRUE(transitionEvents.contains("Stop"));
  EXPECT_TRUE(transitionTargets.contains("Active"));
  EXPECT_TRUE(transitionTargets.contains("Idle"));

  delete fsm;
}
