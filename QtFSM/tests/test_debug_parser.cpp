#include "src/model/FSM.h"
#include "src/parsing/CppParser.h"
#include "src/parsing/Lexer.h"
#include "src/parsing/ModelBuilder.h"
#include <QCoreApplication>
#include <QDebug>


using namespace FSMParser;

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

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

  qDebug() << "=== Testing Parser ===\n";

  // Step 1: Tokenize
  Lexer lexer(testCode);
  QVector<Token> tokens = lexer.tokenize();

  qDebug() << "Tokens around 'MyFSMStateBase':";
  int count = 0;
  for (const Token &token : tokens) {
    if (token.type == TokenType::EndOfFile)
      break;
    qDebug().nospace() << "[" << count++ << "] Line " << token.line << ": "
                       << token.typeName().leftJustified(15) << " = '"
                       << token.value << "'";
    if (count > 20)
      break;
  }

  // Step 2: Parse
  qDebug() << "\n=== Parsing ===";
  CppParser parser(tokens);
  QVector<ClassDecl *> classes = parser.parse();

  if (parser.hasError()) {
    qDebug() << "❌ Parser error:" << parser.errorMessage();
  } else {
    qDebug() << "✅ Parsed" << classes.size() << "classes";
    for (ClassDecl *cls : classes) {
      qDebug() << "  Class:" << cls->name << "(" << cls->methods.size()
               << "methods)";
    }
  }

  qDeleteAll(classes);
  return 0;
}
