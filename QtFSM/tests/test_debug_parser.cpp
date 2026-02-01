#include "../src/model/FSM.h"
#include "../src/parsing/CppParser.h"
#include "../src/parsing/Lexer.h"
#include "../src/parsing/ModelBuilder.h"
#include <QString>
#include <QVector>
#include <gtest/gtest.h>

using namespace FSMParser;

// GTest for Parser Debugging and Low-Level Validation
TEST(ParserDebugTest, TokenizesAndParses BasicCode) {
  QString testCode = R"(
class MyFSMStateBase {
public:
    virtual ~MyFSMStateBase() = default;
};

class State1State : public MyFSMStateBase {
public:
    MyFSMStateBase* handle(MyFSMContext* context, const Event& event) {
        if (event.type == "EVENT") {
            return new State2State();
        }
        return nullptr;
    }
};
)";

  // Step 1: Tokenize
  Lexer lexer(testCode);
  QVector<Token> tokens = lexer.tokenize();

  ASSERT_GT(tokens.size(), 0) << "Lexer should produce tokens";

  // Verify we get EOF at the end
  bool hasEOF = false;
  for (const Token &token : tokens) {
    if (token.type == TokenType::EndOfFile) {
      hasEOF = true;
      break;
    }
  }
  EXPECT_TRUE(hasEOF) << "Token stream should end with EOF";

  // Step 2: Parse
  CppParser parser(tokens);
  QVector<ClassDecl *> classes = parser.parse();

  EXPECT_FALSE(parser.hasError()) << "Parser should not have errors. Error: "
                                  << parser.errorMessage().toStdString();

  ASSERT_EQ(classes.size(), 2) << "Should parse 2 classes";

  // Verify class names
  bool hasBase = false;
  bool hasState1 = false;
  for (ClassDecl *cls : classes) {
    if (cls->name == "MyFSMStateBase")
      hasBase = true;
    if (cls->name == "State1State")
      hasState1 = true;
  }

  EXPECT_TRUE(hasBase) << "Should parse MyFSMStateBase";
  EXPECT_TRUE(hasState1) << "Should parse State1State";

  // Find State1State and verify it has methods
  for (ClassDecl *cls : classes) {
    if (cls->name == "State1State") {
      EXPECT_GT(cls->methods.size(), 0) << "State1State should have methods";
    }
  }

  qDeleteAll(classes);
}
