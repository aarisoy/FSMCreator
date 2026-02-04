cmake_minimum_required(VERSION 3.16)
project(QtFSM VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Add Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Project source files
set(MODEL_SOURCES
    src/model/FSM.cpp
    src/model/State.cpp
    src/model/Transition.cpp
    src/model/Event.cpp
)

set(MODEL_HEADERS
    src/model/FSM.h
    src/model/State.h
    src/model/Transition.h
    src/model/Event.h
)

set(VIEWMODEL_SOURCES
    src/viewmodel/DiagramViewModel.cpp
    src/viewmodel/ProjectViewModel.cpp
    src/viewmodel/commands/AddStateCommand.cpp
    src/viewmodel/commands/DeleteStateCommand.cpp
    src/viewmodel/commands/AddTransitionCommand.cpp
    src/viewmodel/commands/DeleteTransitionCommand.cpp
)

set(VIEWMODEL_HEADERS
    src/viewmodel/DiagramViewModel.h
    src/viewmodel/ProjectViewModel.h
    src/viewmodel/commands/AddStateCommand.h
    src/viewmodel/commands/DeleteStateCommand.h
    src/viewmodel/commands/AddTransitionCommand.h
    src/viewmodel/commands/DeleteTransitionCommand.h
)

set(VIEW_SOURCES
    src/view/MainWindow.cpp
    src/view/DiagramEditor.cpp
    src/view/StateItem.cpp
    src/view/TransitionItem.cpp
    src/view/PropertiesPanel.cpp
    src/view/StateDialog.cpp
    src/view/CodePreviewPanel.cpp
    src/view/TransitionDialog.cpp
    src/view/AboutDialog.cpp
)

set(VIEW_HEADERS
    src/view/MainWindow.h
    src/view/DiagramEditor.h
    src/view/StateItem.h
    src/view/TransitionItem.h
    src/view/PropertiesPanel.h
    src/view/StateDialog.h
    src/view/CodePreviewPanel.h
    src/view/TransitionDialog.h
    src/view/AboutDialog.h
)

set(CODEGEN_SOURCES
    src/codegen/CodeGenerator.cpp
    src/parsing/CodeParser.cpp
    src/parsing/Lexer.cpp
    src/parsing/Token.cpp
    src/parsing/CppParser.cpp
    src/parsing/AST.cpp
    src/parsing/ModelBuilder.cpp
)

set(CODEGEN_HEADERS
    src/codegen/CodeGenerator.h
    src/serialization/JSONSerializer.h
    src/parsing/CodeParser.h
    src/parsing/Lexer.h
    src/parsing/Token.h
    src/parsing/CppParser.h
    src/parsing/AST.h
    src/parsing/ModelBuilder.h
)

set(SERIALIZATION_SOURCES
    src/serialization/JSONSerializer.cpp
)

set(SERIALIZATION_HEADERS
    src/serialization/JSONSerializer.h
)

# Main executable
qt_add_executable(QtFSM
    src/main.cpp
    ${MODEL_SOURCES} ${MODEL_HEADERS}
    ${VIEWMODEL_SOURCES} ${VIEWMODEL_HEADERS}
    ${VIEW_SOURCES} ${VIEW_HEADERS}
    ${CODEGEN_SOURCES} ${CODEGEN_HEADERS}
    ${PARSER_SOURCES} ${PARSER_HEADERS}
    ${SERIALIZATION_SOURCES} ${SERIALIZATION_HEADERS}
)

# Link Qt libraries
target_link_libraries(QtFSM PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Include directories
target_include_directories(QtFSM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Test executable: Stress Test
add_executable(test_stress tests/test_stress.cpp
    ${MODEL_SOURCES} ${MODEL_HEADERS}
    ${PARSING_SOURCES} ${PARSING_HEADERS}
    ${CODEGEN_SOURCES} ${CODEGEN_HEADERS}
    ${SERIALIZATION_SOURCES} ${SERIALIZATION_HEADERS}
)
target_link_libraries(test_stress PRIVATE Qt6::Core GTest::gtest GTest::gtest_main)
target_include_directories(test_stress PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# User Code Test
add_executable(test_user_code tests/test_user_code.cpp
    ${MODEL_SOURCES} ${MODEL_HEADERS}
    ${PARSING_SOURCES} ${PARSING_HEADERS}
    ${CODEGEN_SOURCES} ${CODEGEN_HEADERS}
    ${SERIALIZATION_SOURCES} ${SERIALIZATION_HEADERS}
)
target_link_libraries(test_user_code PRIVATE Qt6::Core GTest::gtest GTest::gtest_main)
target_include_directories(test_user_code PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# JSON Serialization Test
add_executable(test_json tests/test_json.cpp
    ${MODEL_SOURCES} ${MODEL_HEADERS}
    ${PARSING_SOURCES} ${PARSING_HEADERS}
    ${CODEGEN_SOURCES} ${CODEGEN_HEADERS}
    ${SERIALIZATION_SOURCES} ${SERIALIZATION_HEADERS}
)
target_link_libraries(test_json PRIVATE Qt6::Core GTest::gtest GTest::gtest_main)
target_include_directories(test_json PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Lexer Test
add_executable(test_lexer tests/test_lexer.cpp
    ${MODEL_SOURCES} ${MODEL_HEADERS}
    ${PARSING_SOURCES} ${PARSING_HEADERS}
    ${CODEGEN_SOURCES} ${CODEGEN_HEADERS}
    ${SERIALIZATION_SOURCES} ${SERIALIZATION_HEADERS}
)
target_link_libraries(test_lexer PRIVATE Qt6::Core GTest::gtest GTest::gtest_main)
target_include_directories(test_lexer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Parser Test
add_executable(test_parser tests/test_parser.cpp
    ${MODEL_SOURCES} ${MODEL_HEADERS}
    ${PARSING_SOURCES} ${PARSING_HEADERS}
    ${CODEGEN_SOURCES} ${CODEGEN_HEADERS}
    ${SERIALIZATION_SOURCES} ${SERIALIZATION_HEADERS}
)
target_link_libraries(test_parser PRIVATE Qt6::Core GTest::gtest GTest::gtest_main)
target_include_directories(test_parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# FSM Model Test
add_executable(test_fsm tests/test_fsm.cpp
    ${MODEL_SOURCES} ${MODEL_HEADERS}
)
target_link_libraries(test_fsm PRIVATE Qt6::Core GTest::gtest GTest::gtest_main)
target_include_directories(test_fsm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# JSON Roundtrip Test
add_executable(test_json_roundtrip tests/test_json_roundtrip.cpp
    ${MODEL_SOURCES} ${MODEL_HEADERS}
    ${SERIALIZATION_SOURCES} ${SERIALIZATION_HEADERS}
)
target_link_libraries(test_json_roundtrip PRIVATE Qt6::Core GTest::gtest GTest::gtest_main)
target_include_directories(test_json_roundtrip PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Parser Debug Test
add_executable(test_debug_parser tests/test_debug_parser.cpp
    ${MODEL_SOURCES} ${MODEL_HEADERS}
    ${PARSING_SOURCES} ${PARSING_HEADERS}
    ${CODEGEN_SOURCES} ${CODEGEN_HEADERS}
    ${SERIALIZATION_SOURCES} ${SERIALIZATION_HEADERS}
)
target_link_libraries(test_debug_parser PRIVATE Qt6::Core GTest::gtest GTest::gtest_main)
target_include_directories(test_debug_parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Installation
install(TARGETS QtFSM
    RUNTIME DESTINATION bin
)

# Testing (optional, enable with -DBUILD_TESTING=ON)
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()
