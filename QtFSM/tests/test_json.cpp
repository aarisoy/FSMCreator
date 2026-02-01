#include "../src/model/FSM.h"
#include "../src/parsing/CodeParser.h"
#include "../src/serialization/JSONSerializer.h"
#include <QFile>
#include <QSet>
#include <QString>
#include <QTemporaryFile>
#include <gtest/gtest.h>

// GTest for JSON Serialization
TEST(JSONSerializationTest, SaveAndLoadFSM) {
  // Read the input C++ file
  QFile inputFile("../tests/test_input.cpp");
  ASSERT_TRUE(inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
      << "Should be able to open test_input.cpp";

  QString code = inputFile.readAll();
  inputFile.close();

  // Parse the C++ code to FSM
  CodeParser parser;
  FSM *fsm = parser.parse(code);

  ASSERT_NE(fsm, nullptr)
      << "Should successfully parse FSM from test_input.cpp";

  fsm->setName("MyFSM");

  ASSERT_GT(fsm->states().size(), 0) << "Should have at least one state";

  int originalStateCount = fsm->states().size();
  int originalTransitionCount = fsm->transitions().size();

  // Save to temporary JSON file
  QTemporaryFile tempFile;
  ASSERT_TRUE(tempFile.open()) << "Should create temporary file";
  QString tempFileName = tempFile.fileName();
  tempFile.close();

  JSONSerializer serializer;
  ASSERT_TRUE(serializer.save(fsm, tempFileName))
      << "Should successfully save FSM to JSON";

  // Load from JSON
  FSM *loadedFsm = serializer.load(tempFileName);

  ASSERT_NE(loadedFsm, nullptr) << "Should successfully load FSM from JSON";

  // Verify FSM name
  EXPECT_EQ(loadedFsm->name(), "MyFSM")
      << "Loaded FSM should have correct name";

  // Verify state count matches
  EXPECT_EQ(loadedFsm->states().size(), originalStateCount)
      << "Loaded FSM should have same number of states";

  // Verify transition count matches
  EXPECT_EQ(loadedFsm->transitions().size(), originalTransitionCount)
      << "Loaded FSM should have same number of transitions";

  // Verify state names match
  QSet<QString> originalStateNames;
  for (State *state : fsm->states()) {
    originalStateNames.insert(state->name());
  }

  QSet<QString> loadedStateNames;
  for (State *state : loadedFsm->states()) {
    loadedStateNames.insert(state->name());
  }

  EXPECT_EQ(loadedStateNames, originalStateNames)
      << "Loaded FSM should have same state names";

  // Clean up
  delete fsm;
  delete loadedFsm;
  QFile::remove(tempFileName);
}
