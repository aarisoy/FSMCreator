#include "../src/model/FSM.h"
#include "../src/parsing/CodeParser.h"
#include <QString>
#include <gtest/gtest.h>


// GTest for FSM Parser Stress Test
TEST(FSMParserStressTest, HandlesComplexCode) {
  QString trickyCode = R"(
    #include <string>
    
    // 1. Forward declaration (should be skipped)
    class SomeForwardClass;
    
    // 2. Class with "State" in name but not ending in "State" (should be ignored)
    class StateMachineConfig {
        int x;
    };

    // 3. Valid State with weird formatting
    class   WeirdFormatState    :    public   MyFSMStateBase    {
    private:
        // Member that looks like function
        MyFSMStateBase*   somePtr ;
        void* (*functionPtr)(int); // Complex function pointer member
        
    public:
        // Destructor with spacing
        virtual    ~   WeirdFormatState  ( ) { }
        
        // Constructor
        WeirdFormatState() : somePtr(nullptr) {}

        MyFSMStateBase* handle(MyFSMContext* ctx, const Event& evt) override {
            
            // False positive string
            // std::string s = "return new FakeState();"; 
            
            // Transition 1: Weird spacing in if
            if (   evt.type    ==    "SpaceEvent"   ) {
                return      new       TargetOneState();
            }
            
            // Transition 2: Commented out transition (should be ignored)
            /* 
            if (evt.type == "FakeEvent") {
                return new FakeTarget();
            }
            */
            
            // Transition 3: Normal transition
            if (evt.type == "GoToTwo") {
                return new TargetTwoState();
            }
            
            // Transition 4: No space before parenthesis
            if(evt.type=="Compact"){
                return new CompactState();
            }
            
            // Edge case: Transition inside nested braces
            {
                if (evt.type == "Nested") {
                    return new NestedTargetState();
                }
            }
            
            return nullptr;
        }
    };
    
    // 4. State with no transitions
    class   NoTransitionsState  :  public MyFSMStateBase {
    public:
        MyFSMStateBase* handle(MyFSMContext* ctx, const Event& evt) override {
            // No transitions
            return nullptr;
        }
    };
    )";

  // Parse the code
  CodeParser parser;
  FSM *fsm = parser.parse(trickyCode);

  // Validate FSM
  ASSERT_NE(fsm, nullptr) << "FSM should be created from tricky code";

  // Should detect 2 States (WeirdFormatState, NoTransitionsState)
  // StateMachineConfig should be skipped
  EXPECT_EQ(fsm->states().size(), 2) << "Should have exactly 2 states";

  // Find WeirdFormatState
  State *weirdState = nullptr;
  for (auto state : fsm->states()) {
    if (state->name() == "WeirdFormatState") {
      weirdState = state;
      break;
    }
  }

  ASSERT_NE(weirdState, nullptr) << "WeirdFormatState should be found";

  // WeirdFormatState should have 4 transitions (non-commented ones)
  EXPECT_EQ(weirdState->transitions().size(), 4)
      << "WeirdFormatState should have 4 transitions";

  // Verify transition targets
  bool hasTargetOne = false;
  bool hasTargetTwo = false;
  bool hasCompact = false;
  bool hasNested = false;

  for (auto trans : weirdState->transitions()) {
    if (trans->targetState()->name() == "TargetOneState") {
      hasTargetOne = true;
    } else if (trans->targetState()->name() == "TargetTwoState") {
      hasTargetTwo = true;
    } else if (trans->targetState()->name() == "CompactState") {
      hasCompact = true;
    } else if (trans->targetState()->name() == "NestedTargetState") {
      hasNested = true;
    }
  }

  EXPECT_TRUE(hasTargetOne) << "Should have transition to TargetOneState";
  EXPECT_TRUE(hasTargetTwo) << "Should have transition to TargetTwoState";
  EXPECT_TRUE(hasCompact) << "Should have transition to CompactState";
  EXPECT_TRUE(hasNested) << "Should have transition to NestedTargetState";

  // Check NoTransitionsState
  State *noTransState = nullptr;
  for (auto state : fsm->states()) {
    if (state->name() == "NoTransitionsState") {
      noTransState = state;
      break;
    }
  }

  ASSERT_NE(noTransState, nullptr) << "NoTransitionsState should be found";
  EXPECT_EQ(noTransState->transitions().size(), 0)
      << "NoTransitionsState should have no transitions";

  delete fsm;
}
