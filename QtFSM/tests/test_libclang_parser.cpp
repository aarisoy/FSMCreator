#include "../src/model/FSM.h"
#include "../src/model/State.h"
#include "../src/model/Transition.h"
#include "../src/parsing/CodeParser.h"
#include <QSet>
#include <QString>
#include <gtest/gtest.h>

#if defined(FSM_ENABLE_LIBCLANG)

TEST(LibClangParserTest, ParsesQualifiedEnumAndCallExpressions) {
  qputenv("FSM_USE_LIBCLANG", "1");

  QString testCode = R"(
enum class EventType { Timeout, Start };

EventType getEventType();

struct Event {
    EventType type;
};

class MyFSMStateBase {
public:
    virtual ~MyFSMStateBase() = default;
    virtual MyFSMStateBase* handle(MyFSMContext* context, const Event& event) = 0;
    virtual std::string getName() const = 0;
};

class IdleState : public MyFSMStateBase {
public:
    MyFSMStateBase* handle(MyFSMContext* context, const Event& event) override {
        if (event.type == EventType::Timeout) {
            return new ActiveState();
        }
        if (event.type == getEventType()) {
            return new IdleState();
        }
        return nullptr;
    }

    std::string getName() const override { return "Idle"; }
};

class ActiveState : public MyFSMStateBase {
public:
    MyFSMStateBase* handle(MyFSMContext* context, const Event& event) override {
        return nullptr;
    }
    std::string getName() const override { return "Active"; }
};
)";

  CodeParser parser;
  FSM *fsm = parser.parse(testCode);

  ASSERT_NE(fsm, nullptr);
  ASSERT_EQ(fsm->states().size(), 2);

  State *idle = fsm->stateById("Idle");
  ASSERT_NE(idle, nullptr);

  QSet<QString> events;
  for (Transition *transition : idle->transitions()) {
    events.insert(transition->event());
  }

  EXPECT_TRUE(events.contains("EventType::Timeout"));
  EXPECT_TRUE(events.contains("getEventType"));

  delete fsm;
  qputenv("FSM_USE_LIBCLANG", "0");
}

#endif
