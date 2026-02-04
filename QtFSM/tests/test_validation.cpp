#include "../src/model/FSM.h"
#include "../src/model/State.h"
#include "../src/model/Transition.h"
#include <gtest/gtest.h>

class ValidationTest : public ::testing::Test {
protected:
  void SetUp() override {
    fsm = new FSM();
    fsm->setName("TestFSM");
  }

  void TearDown() override { delete fsm; }

  FSM *fsm;
};

TEST_F(ValidationTest, ValidateEmptyFSM) {
  QString error;
  EXPECT_FALSE(fsm->validate(&error));
  EXPECT_EQ(error, "FSM must have at least one state");
}

TEST_F(ValidationTest, ValidateNoInitialState) {
  State *s1 = new State("s1", "S1", fsm);
  fsm->addState(s1);

  QString error;
  EXPECT_FALSE(fsm->validate(&error));
  EXPECT_EQ(error, "FSM must have an initial state");
}

TEST_F(ValidationTest, ValidateDuplicateStateIDs) {
  State *s1 = new State("s1", "S1", fsm); // ID "s1"
  fsm->addState(s1);
  fsm->setInitialState(s1);

  State *s2 = new State("s1", "Duplicate S1", fsm); // ID "s1" again
  fsm->addState(s2);

  QString error;
  EXPECT_FALSE(fsm->validate(&error));
  EXPECT_TRUE(error.contains("Duplicate state ID found"));
}

TEST_F(ValidationTest, ValidateDuplicateTransitionIDs) {
  State *s1 = new State("s1", "S1", fsm);
  State *s2 = new State("s2", "S2", fsm);
  fsm->addState(s1);
  fsm->addState(s2);
  fsm->setInitialState(s1);

  Transition *t1 = new Transition("t1", s1, s2, fsm);
  fsm->addTransition(t1);

  Transition *t2 = new Transition("t1", s2, s1, fsm); // Duplicate ID "t1"
  fsm->addTransition(t2);

  QString error;
  EXPECT_FALSE(fsm->validate(&error));
  EXPECT_TRUE(error.contains("Duplicate transition ID found"));
}

TEST_F(ValidationTest, ValidateUnreachableState) {
  State *s1 = new State("start", "Start", fsm);
  State *s2 = new State("unreachable", "Unreachable", fsm);
  fsm->addState(s1);
  fsm->addState(s2);
  fsm->setInitialState(s1);

  // No transition to s2

  QString error;
  EXPECT_FALSE(fsm->validate(&error));
  EXPECT_TRUE(error.contains("unreachable"));
}

TEST_F(ValidationTest, ValidateValidSimpleFSM) {
  State *s1 = new State("start", "Start", fsm);
  State *s2 = new State("end", "End", fsm);
  fsm->addState(s1);
  fsm->addState(s2);
  fsm->setInitialState(s1);

  Transition *t1 = new Transition("t1", s1, s2, fsm);
  fsm->addTransition(t1);

  QString error;
  EXPECT_TRUE(fsm->validate(&error));
  EXPECT_TRUE(error.isEmpty());
}
