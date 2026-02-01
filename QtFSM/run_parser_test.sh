# Build the parser test
cd build
g++ -std=c++17 -I/usr/include/x86_64-linux-gnu/qt6 -I/usr/include/x86_64-linux-gnu/qt6/QtCore \
    -I../src \
    ../test_parser.cpp \
    ../src/parsing/CodeParser.cpp \
    ../src/model/FSM.cpp \
    ../src/model/State.cpp \
    ../src/model/Transition.cpp \
    ../src/model/Event.cpp \
    -lQt6Core \
    -fPIC \
    -o test_parser

# Run the test
./test_parser
