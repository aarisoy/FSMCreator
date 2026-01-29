cmake_minimum_required(VERSION 3.16)
project(QtFSM VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

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
)

set(VIEWMODEL_HEADERS
    src/viewmodel/DiagramViewModel.h
    src/viewmodel/ProjectViewModel.h
    src/viewmodel/commands/AddStateCommand.h
    src/viewmodel/commands/DeleteStateCommand.h
    src/viewmodel/commands/AddTransitionCommand.h
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
)

set(CODEGEN_SOURCES
    src/codegen/CodeGenerator.cpp
    src/serialization/JSONSerializer.cpp
    src/parsing/CodeParser.cpp
)

set(CODEGEN_HEADERS
    src/codegen/CodeGenerator.h
    src/serialization/JSONSerializer.h
    src/parsing/CodeParser.h
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
