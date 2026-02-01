#include "../src/parsing/Lexer.h"
#include <QString>
#include <gtest/gtest.h>

using namespace FSMParser;

// GTest for Lexer Tokenization
TEST(LexerTest, TokenizesSimpleStateMachine) {
  QString testCode = R"(
class State1State : public BaseState {
public:
    BaseState* handle(Context* ctx, const Event& event) {
        if (event.type == "Click") {
            return new State2State();
        }
        return nullptr;
    }
};
)";

  Lexer lexer(testCode);
  QVector<Token> tokens = lexer.tokenize();

  ASSERT_GT(tokens.size(), 0) << "Lexer should produce tokens";

  // Count non-EOF tokens
  int count = 0;
  for (const Token &token : tokens) {
    if (token.type == TokenType::EndOfFile)
      break;
    count++;
  }

  EXPECT_GT(count, 10)
      << "Should have multiple tokens for the state machine code";

  // Verify important keywords are tokenized
  bool hasClass = false;
  bool hasPublic = false;
  bool hasIf = false;
  bool hasReturn = false;

  for (const Token &token : tokens) {
    if (token.type == TokenType::EndOfFile)
      break;
    if (token.value == "class")
      hasClass = true;
    if (token.value == "public")
      hasPublic = true;
    if (token.value == "if")
      hasIf = true;
    if (token.value == "return")
      hasReturn = true;
  }

  EXPECT_TRUE(hasClass) << "Should tokenize 'class' keyword";
  EXPECT_TRUE(hasPublic) << "Should tokenize 'public' keyword";
  EXPECT_TRUE(hasIf) << "Should tokenize 'if' keyword";
  EXPECT_TRUE(hasReturn) << "Should tokenize 'return' keyword";
}
