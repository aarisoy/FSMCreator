#include "src/parsing/Lexer.h"
#include <QCoreApplication>
#include <QDebug>

using namespace FSMParser;

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

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

  qDebug() << "=== Lexer Test ===\n";
  qDebug() << "Input code:";
  qDebug() << testCode;
  qDebug() << "\n=== Tokens ===";

  Lexer lexer(testCode);
  QVector<Token> tokens = lexer.tokenize();

  int count = 0;
  for (const Token &token : tokens) {
    if (token.type == TokenType::EndOfFile)
      break;
    qDebug().nospace() << "[" << count++ << "] "
                       << token.typeName().leftJustified(15) << " : '"
                       << token.value << "'";
    if (count > 50) {
      qDebug() << "... (truncated)";
      break;
    }
  }

  qDebug() << "\n✅ Lexer test complete! Total tokens:" << count;
  return 0;
}
