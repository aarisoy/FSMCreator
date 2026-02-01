#include "../src/model/FSM.h"
#include "../src/model/State.h"
#include "../src/model/Transition.h"
#include "../src/parsing/CodeParser.h"
#include <QDebug>
#include <QString>

// Note: FSM, State, Transition, CodeParser are in global namespace
// internal parser classes (Lexer, Parser, AST) are in FSMParser namespace

void runStressTest() {
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
            
            // Transition 3: String literal with quotes
            if (evt.type == "Quote\"Event") {
                return new TargetTwoState();
            }
            
            return nullptr;
        }
        
        std::string getName() const override { return "WeirdFormat"; }
    };

    // 4. Another valid state to be the target
    class TargetOneState : public MyFSMStateBase { 
        MyFSMStateBase* handle(MyFSMContext* c, const Event& e) override { return nullptr; }
        std::string getName() const override { return "Target1"; }
    };
    
    class TargetTwoState : public MyFSMStateBase {
        MyFSMStateBase* handle(MyFSMContext* c, const Event& e) override { return nullptr; }
        std::string getName() const override { return "Target2"; }
    };
    )";

  qDebug() << "=== Starting Stress Test ===";

  CodeParser parser;
  FSM *fsm = parser.parse(trickyCode);

  if (fsm) {
    qDebug() << "Parsed Successfully!";
    qDebug() << "State Count:" << fsm->states().size();

    // QList iteration
    foreach (State *s, fsm->states()) {
      if (!s)
        continue;
      qDebug() << "State:" << s->name();
      foreach (Transition *t, s->transitions()) {
        if (!t)
          continue;
        qDebug() << "  Transition:" << t->event() << "->"
                 << (t->targetState() ? t->targetState()->name() : "NULL");
      }
    }
  } else {
    qDebug() << "Parsing Failed:" << parser.lastError();
  }
}

int main() {
  runStressTest();
  return 0;
}
