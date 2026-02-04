#include "../src/model/FSM.h"
#include "../src/model/State.h"
#include "../src/model/Transition.h"
#include "../src/serialization/JSONSerializer.h"
#include <QFile>
#include <QTemporaryFile>
#include <gtest/gtest.h>


class JSONRoundtripTest : public ::testing::Test {
protected:
  void SetUp() override {
    fsm = new FSM();
    fsm->setName("TestFSM");
  }

  void TearDown() override { delete fsm; }

  FSM *fsm;
};

// Test complete roundtrip of all model fields
TEST_F(JSONRoundtripTest, RoundtripAllFields) {
  // 1. Setup complex FSM
  State *s1 = new State("start", "Start", fsm);
  s1->setPosition(QPointF(10, 20));
  s1->setInitial(true);
  s1->setEntryAction("log('Enter Start');");
  s1->setExitAction("log('Exit Start');");
  s1->addFunction("void init()");
  fsm->addState(s1);

  State *s2 = new State("process", "Process", fsm);
  s2->setPosition(QPointF(100, 200));
  s2->setFinal(true);
  s2->addFunction("int compute(int x)");
  fsm->addState(s2);

  Transition *t1 = new Transition("t1", s1, s2, fsm);
  t1->setEvent("start_event");
  t1->setGuard("x > 0");
  t1->setAction("doWork()");
  fsm->addTransition(t1);

  // 2. Save to temporary file
  QTemporaryFile tempFile;
  ASSERT_TRUE(tempFile.open());
  QString filePath = tempFile.fileName();
  tempFile.close(); // Close so serializer can write to it

  JSONSerializer serializer;
  ASSERT_TRUE(serializer.save(fsm, filePath));

  // 3. Load back
  FSM *loadedFsm = serializer.load(filePath);
  ASSERT_NE(loadedFsm, nullptr);

  // 4. Verify all fields match
  EXPECT_EQ(loadedFsm->name(), "TestFSM");
  EXPECT_EQ(loadedFsm->states().size(), 2);
  EXPECT_EQ(loadedFsm->transitions().size(), 1);

  // Verify State 1
  State *loadedS1 = nullptr;
  for (auto s : loadedFsm->states()) {
    if (s->id() == "start")
      loadedS1 = s;
  }
  ASSERT_NE(loadedS1, nullptr);
  EXPECT_EQ(loadedS1->name(), "Start");
  EXPECT_EQ(loadedS1->position().x(), 10);
  EXPECT_EQ(loadedS1->position().y(), 20);
  EXPECT_TRUE(loadedS1->isInitial());
  EXPECT_FALSE(loadedS1->isFinal()); // Explicit check
  EXPECT_EQ(loadedS1->entryAction(), "log('Enter Start');");
  EXPECT_EQ(loadedS1->exitAction(), "log('Exit Start');");
  ASSERT_EQ(loadedS1->customFunctions().size(), 1);
  EXPECT_EQ(loadedS1->customFunctions()[0], "void init()");

  // Verify State 2
  State *loadedS2 = nullptr;
  for (auto s : loadedFsm->states()) {
    if (s->id() == "process")
      loadedS2 = s;
  }
  ASSERT_NE(loadedS2, nullptr);
  EXPECT_EQ(loadedS2->name(), "Process");
  EXPECT_TRUE(loadedS2->isFinal());
  EXPECT_FALSE(loadedS2->isInitial()); // Explicit check
  ASSERT_EQ(loadedS2->customFunctions().size(), 1);
  EXPECT_EQ(loadedS2->customFunctions()[0], "int compute(int x)");

  // Verify Transition
  Transition *loadedT1 = loadedFsm->transitions()[0];
  ASSERT_NE(loadedT1, nullptr);
  EXPECT_EQ(loadedT1->id(), "t1");
  EXPECT_EQ(loadedT1->sourceState()->id(), "start");
  EXPECT_EQ(loadedT1->targetState()->id(), "process");
  EXPECT_EQ(loadedT1->event(), "start_event");
  EXPECT_EQ(loadedT1->guard(), "x > 0");
  EXPECT_EQ(loadedT1->action(), "doWork()");

  delete loadedFsm;
}
