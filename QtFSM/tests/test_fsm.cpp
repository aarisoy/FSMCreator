#include "../src/model/FSM.h"
#include "../src/model/State.h"
#include "../src/model/Transition.h"
#include <gtest/gtest.h>

// Test that removing a state also removes its related transitions
TEST(FSMTest, RemoveStateCleanupTransitions) {
  FSM fsm;

  // Create states
  State *state1 = new State("state1", "State1", &fsm);
  State *state2 = new State("state2", "State2", &fsm);
  State *state3 = new State("state3", "State3", &fsm);

  fsm.addState(state1);
  fsm.addState(state2);
  fsm.addState(state3);

  // Create transitions
  Transition *trans1 = new Transition(state1, state2, &fsm); // state1 -> state2
  Transition *trans2 = new Transition(state2, state3, &fsm); // state2 -> state3
  Transition *trans3 = new Transition(state3, state1, &fsm); // state3 -> state1

  trans1->setEvent("event1");
  trans2->setEvent("event2");
  trans3->setEvent("event3");

  fsm.addTransition(trans1);
  fsm.addTransition(trans2);
  fsm.addTransition(trans3);

  // Verify initial state
  ASSERT_EQ(fsm.states().size(), 3);
  ASSERT_EQ(fsm.transitions().size(), 3);

  // Remove state2 - should also remove trans1 and trans2
  fsm.removeState(state2);

  // Verify state2 is removed
  EXPECT_EQ(fsm.states().size(), 2);
  EXPECT_TRUE(fsm.states().contains(state1));
  EXPECT_FALSE(fsm.states().contains(state2));
  EXPECT_TRUE(fsm.states().contains(state3));

  // CRITICAL: Verify transitions referencing state2 are also removed
  EXPECT_EQ(fsm.transitions().size(), 1)
      << "Transitions referencing deleted state should be removed";
  EXPECT_TRUE(fsm.transitions().contains(trans3))
      << "Transition not referencing deleted state should remain";
  EXPECT_FALSE(fsm.transitions().contains(trans1))
      << "Transition from deleted state should be removed";
  EXPECT_FALSE(fsm.transitions().contains(trans2))
      << "Transition to deleted state should be removed";
}

// Test removing initial state
TEST(FSMTest, RemoveInitialState) {
  FSM fsm;

  State *initialState = new State("initial", "Initial", &fsm);
  State *otherState = new State("other", "Other", &fsm);

  fsm.addState(initialState);
  fsm.addState(otherState);
  fsm.setInitialState(initialState);

  ASSERT_EQ(fsm.initialState(), initialState);

  // Remove initial state
  fsm.removeState(initialState);

  // Verify initial state is cleared
  EXPECT_EQ(fsm.initialState(), nullptr)
      << "Initial state pointer should be cleared when state is removed";
  EXPECT_EQ(fsm.states().size(), 1);
}

// Test removing state with no transitions
TEST(FSMTest, RemoveIsolatedState) {
  FSM fsm;

  State *state1 = new State("state1", "State1", &fsm);
  State *state2 = new State("state2", "State2", &fsm);

  fsm.addState(state1);
  fsm.addState(state2);

  // No transitions created

  ASSERT_EQ(fsm.states().size(), 2);
  ASSERT_EQ(fsm.transitions().size(), 0);

  // Remove state1
  fsm.removeState(state1);

  // Should work fine with no transitions
  EXPECT_EQ(fsm.states().size(), 1);
  EXPECT_EQ(fsm.transitions().size(), 0);
  EXPECT_TRUE(fsm.states().contains(state2));
}
