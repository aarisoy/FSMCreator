#include "MainWindow.h"
#include "../codegen/CodeGenerator.h"
#include "../model/FSM.h"
#include "../parsing/CodeParser.h"
#include "../serialization/JSONSerializer.h"
#include "../viewmodel/DiagramViewModel.h"
#include "AboutDialog.h"
#include "CodePreviewPanel.h"
#include "DiagramEditor.h"
#include "PropertiesPanel.h"
#include "StateItem.h"
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QFont>
#include <QGraphicsItem>
#include <QInputDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QStyle>
#include <QTextStream>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_diagramEditor(nullptr), m_viewModel(nullptr),
      m_propertiesPanel(nullptr), m_darkTheme(false) {
  setupUi();
  createActions();
  createMenus();
  createToolBars();
  createDockWidgets();

  setWindowTitle("QtFSM Designer");
  resize(1400, 900);

  // Apply initial theme
  applyTheme();

  // Add status bar
  statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
  // Create central diagram editor
  m_diagramEditor = new DiagramEditor(this);
  setCentralWidget(m_diagramEditor);

  // Create ViewModel with FSM
  m_viewModel = new DiagramViewModel(this);
  FSM *fsm = new FSM(this);
  m_viewModel->setFSM(fsm);

  // Connect editor to ViewModel
  m_diagramEditor->setViewModel(m_viewModel);

  // Connect diagram changes to code preview
  // Connect diagram changes to code preview
  // Manual Mode: Disable auto-update
  // connect(m_diagramEditor, &DiagramEditor::fsmChanged, this,
  // &MainWindow::updateCodePreview);

  // Create properties panel
  m_propertiesPanel = new PropertiesPanel(this);
  m_propertiesPanel->setFSM(fsm); // Connect to initial FSM

  // Create code preview panel
  m_codePreviewPanel = new CodePreviewPanel(this);
  connect(m_codePreviewPanel, &CodePreviewPanel::updateDiagramRequested, this,
          &MainWindow::updateDiagramFromCode);
  connect(m_codePreviewPanel, &CodePreviewPanel::generateCodeRequested, this,
          &MainWindow::updateCodePreview);
}

void MainWindow::createActions() {
  // New action
  m_newAction = new QAction(tr("&New"), this);
  m_newAction->setShortcuts(QKeySequence::New);
  m_newAction->setStatusTip(tr("Create a new FSM project"));
  connect(m_newAction, &QAction::triggered, this, &MainWindow::newProject);

  // Import JSON action
  m_importJsonAction = new QAction(tr("Import &JSON..."), this);
  m_importJsonAction->setShortcuts(QKeySequence::Open);
  m_importJsonAction->setStatusTip(tr("Import FSM from JSON file"));
  connect(m_importJsonAction, &QAction::triggered, this,
          &MainWindow::importJson);

  // Import C++ action
  m_importCppAction = new QAction(tr("Import C&++..."), this);
  m_importCppAction->setStatusTip(tr("Import FSM from C++ config file"));
  connect(m_importCppAction, &QAction::triggered, this, &MainWindow::importCpp);

  // Save action
  m_saveAction = new QAction(tr("&Save"), this);
  m_saveAction->setShortcuts(QKeySequence::Save);
  m_saveAction->setStatusTip(tr("Save the current FSM project"));
  connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveProject);

  // Export C++ action
  m_exportAction = new QAction(tr("Export C++ Config..."), this);
  m_exportAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
  m_exportAction->setStatusTip(tr("Generate C++ config from the FSM"));
  connect(m_exportAction, &QAction::triggered, this, &MainWindow::exportCpp);

  // Export JSON action
  m_exportJsonAction = new QAction(tr("Export JSON..."), this);
  m_exportJsonAction->setShortcut(
      QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E));
  m_exportJsonAction->setStatusTip(tr("Export FSM to JSON format"));
  connect(m_exportJsonAction, &QAction::triggered, this,
          &MainWindow::exportJson);

  // Exit action
  m_exitAction = new QAction(tr("E&xit"), this);
  m_exitAction->setShortcuts(QKeySequence::Quit);
  m_exitAction->setStatusTip(tr("Exit the application"));
  connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

  // Undo action
  m_undoAction = new QAction(tr("&Undo"), this);
  m_undoAction->setShortcuts(QKeySequence::Undo);
  m_undoAction->setStatusTip(tr("Undo the last action"));
  m_undoAction->setEnabled(false);
  connect(m_undoAction, &QAction::triggered, m_viewModel,
          &DiagramViewModel::undo);
  connect(m_viewModel, &DiagramViewModel::undoRedoStateChanged, this,
          [this]() { m_undoAction->setEnabled(m_viewModel->canUndo()); });

  // Redo action
  m_redoAction = new QAction(tr("&Redo"), this);
  m_redoAction->setShortcuts(QKeySequence::Redo);
  m_redoAction->setStatusTip(tr("Redo the last undone action"));
  m_redoAction->setEnabled(false);
  connect(m_redoAction, &QAction::triggered, m_viewModel,
          &DiagramViewModel::redo);
  connect(m_viewModel, &DiagramViewModel::undoRedoStateChanged, this,
          [this]() { m_redoAction->setEnabled(m_viewModel->canRedo()); });

  // Toggle theme action
  m_toggleThemeAction = new QAction(tr("Dark Theme"), this);
  m_toggleThemeAction->setCheckable(true);
  m_toggleThemeAction->setChecked(false);
  m_toggleThemeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
  m_toggleThemeAction->setStatusTip(tr("Toggle between light and dark theme"));
  connect(m_toggleThemeAction, &QAction::triggered, this,
          &MainWindow::toggleTheme);

  // About action
  m_aboutAction = new QAction(tr("About QtFSM"), this);
  m_aboutAction->setStatusTip(tr("About this application"));
  connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::createMenus() {
  // File menu
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(m_newAction);
  fileMenu->addAction(m_importJsonAction);
  fileMenu->addAction(m_importCppAction);
  fileMenu->addAction(m_saveAction);
  fileMenu->addSeparator();
  fileMenu->addAction(m_exportAction);
  fileMenu->addAction(m_exportJsonAction);
  fileMenu->addSeparator();
  fileMenu->addAction(m_exitAction);

  // Edit menu
  QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(m_undoAction);
  editMenu->addAction(m_redoAction);

  QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(m_toggleThemeAction);

  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(m_aboutAction);
}

void MainWindow::createToolBars() {
  QToolBar *fileToolBar = addToolBar(tr("File"));
  fileToolBar->setIconSize(QSize(24, 24));
  fileToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  fileToolBar->addAction(m_newAction);
  fileToolBar->addAction(m_saveAction);

  addToolBarBreak();

  QToolBar *editToolBar = addToolBar(tr("Edit"));
  editToolBar->setIconSize(QSize(24, 24));
  editToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  // Add state button
  QAction *addStateAction = new QAction(tr("Add State"), this);
  addStateAction->setToolTip(
      tr("Add a new state to the diagram (Shortcut: Ctrl+N)"));
  addStateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
  connect(addStateAction, &QAction::triggered, m_diagramEditor,
          &DiagramEditor::addState);
  editToolBar->addAction(addStateAction);

  // Add transition button
  QAction *addTransitionAction = new QAction(tr("Add Transition"), this);
  addTransitionAction->setToolTip(tr(
      "Connect states with a transition\n(Select source, then target state)"));
  connect(addTransitionAction, &QAction::triggered, m_diagramEditor,
          &DiagramEditor::startTransitionMode);
  editToolBar->addAction(addTransitionAction);

  editToolBar->addSeparator();

  // Delete button
  QAction *deleteAction = new QAction(tr("Delete"), this);
  deleteAction->setShortcut(QKeySequence::Delete);
  deleteAction->setToolTip(tr("Delete selected items"));
  connect(deleteAction, &QAction::triggered, m_diagramEditor,
          &DiagramEditor::deleteSelected);
  editToolBar->addAction(deleteAction);

  editToolBar->addSeparator();

  // Undo button
  editToolBar->addAction(m_undoAction);

  // Redo button
  editToolBar->addAction(m_redoAction);
}

void MainWindow::createDockWidgets() {
  // Properties dock widget (right side)
  QDockWidget *propertiesDock = new QDockWidget(tr("Properties"), this);
  propertiesDock->setWidget(m_propertiesPanel);
  addDockWidget(Qt::RightDockWidgetArea, propertiesDock);

  // Code preview dock widget (bottom)
  QDockWidget *codePreviewDock =
      new QDockWidget(tr("Generated C++ Config"), this);
  codePreviewDock->setWidget(m_codePreviewPanel);
  addDockWidget(Qt::BottomDockWidgetArea, codePreviewDock);

  // Set minimum height for code preview
  codePreviewDock->setMinimumHeight(200);
}

void MainWindow::updateCodePreview() {
  if (m_diagramEditor && m_diagramEditor->fsm()) {
    m_codePreviewPanel->updateCode(m_diagramEditor->fsm());
    statusBar()->showMessage(tr("Code generated from FSM model"), 1000);
  } else {
    m_codePreviewPanel->clearCode();
  }
}

// File Operations
void MainWindow::newProject() {
  // Get FSM name from user
  bool ok;
  QString fsmName =
      QInputDialog::getText(this, tr("New FSM Project"), tr("Enter FSM name:"),
                            QLineEdit::Normal, "MyFSM", &ok);
  if (!ok || fsmName.isEmpty()) {
    return;
  }

  // Create new FSM
  FSM *fsm = new FSM(this);
  fsm->setName(fsmName);
  m_diagramEditor->setFSM(fsm);
  m_propertiesPanel->setFSM(fsm);

  // Disable automatic updates for "Manual Mode"
  // connect(fsm, &FSM::modified, this, &MainWindow::updateCodePreview);

  // Initial window title
  connect(fsm, &FSM::nameChanged, this, [this](const QString &name) {
    setWindowTitle(QString("QtFSM Designer - %1").arg(name));
  });

  m_currentFile.clear();
  setWindowTitle(QString("QtFSM Designer - %1").arg(fsmName));
  statusBar()->showMessage(tr("New project created: %1").arg(fsmName));
}

void MainWindow::importJson() {
  QString selectedFilter = tr("JSON Files (*.json)");
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Import FSM from JSON"), "",
      tr("JSON Files (*.json);;All Files (*)"), &selectedFilter);

  if (fileName.isEmpty()) {
    return;
  }

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(
        this, tr("Error"),
        tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
    return;
  }

  // Load JSON using JSONSerializer
  JSONSerializer serializer;
  FSM *loadedFsm = serializer.load(fileName);

  if (!loadedFsm) {
    QMessageBox::warning(this, tr("Error"),
                         tr("Failed to load FSM from %1.").arg(fileName));
    return;
  }

  // Set loaded FSM to the editor
  m_diagramEditor->setFSM(loadedFsm);
  m_propertiesPanel->setFSM(loadedFsm);

  // Update ViewModel
  m_viewModel->setFSM(loadedFsm);

  m_currentFile = fileName;
  setWindowTitle(QString("QtFSM Designer - %1").arg(loadedFsm->name()));
  statusBar()->showMessage(tr("Loaded: %1").arg(fileName), 3000);
}

void MainWindow::importCpp() {
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Import C++ Code"), "",
      tr("C++ Files (*.cpp *.h *.hpp);;All Files (*)"));

  if (fileName.isEmpty()) {
    return;
  }

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::warning(
        this, tr("Error"),
        tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
    return;
  }

  // Read the C++ config code
  QString cppCode = file.readAll();
  file.close();

  // Set the code in the code preview panel
  m_codePreviewPanel->setCode(cppCode);

  statusBar()->showMessage(
      tr("C++ config loaded. Click 'Update Diagram' to parse."), 5000);
}

void MainWindow::saveProject() {
  FSM *fsm = m_diagramEditor->fsm();

  if (!fsm || fsm->states().isEmpty()) {
    QMessageBox::warning(this, tr("No FSM to Save"),
                         tr("Please create at least one state before saving."));
    return;
  }

  // If no current file, treat as "Save As"
  if (m_currentFile.isEmpty()) {
    saveAsProject();
    return;
  }

  // Save using JSONSerializer
  JSONSerializer serializer;
  if (!serializer.save(fsm, m_currentFile)) {
    QMessageBox::critical(
        this, tr("Save Failed"),
        tr("Failed to save project to %1.").arg(m_currentFile));
    return;
  }

  statusBar()->showMessage(tr("Project saved: %1").arg(m_currentFile), 3000);
}

void MainWindow::saveAsProject() {
  FSM *fsm = m_diagramEditor->fsm();

  if (!fsm || fsm->states().isEmpty()) {
    QMessageBox::warning(this, tr("No FSM to Save"),
                         tr("Please create at least one state before saving."));
    return;
  }

  // Always ask for a file path (this is "Save As")
  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Save FSM Project As"), "",
      tr("FSM Project Files (*.json);;All Files (*)"));

  if (fileName.isEmpty()) {
    return;
  }

  // Ensure .json extension
  if (!fileName.endsWith(".json", Qt::CaseInsensitive)) {
    fileName += ".json";
  }

  // Save using JSONSerializer
  JSONSerializer serializer;
  if (!serializer.save(fsm, fileName)) {
    QMessageBox::critical(this, tr("Save Failed"),
                          tr("Failed to save project to %1.").arg(fileName));
    return;
  }

  // Update current file
  m_currentFile = fileName;
  statusBar()->showMessage(tr("Project saved: %1").arg(m_currentFile), 3000);
}

void MainWindow::exportCpp() {
  if (!m_diagramEditor->fsm() || m_diagramEditor->fsm()->states().isEmpty()) {
    QMessageBox::warning(
        this, tr("No FSM"),
        tr("Please create at least one state before exporting.\n\nUse 'New' to "
           "create a project and 'Add State' to add states."));
    return;
  }

  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Export C++ Config"), "",
      tr("C++ Header Files (*.h);;C++ Source Files (*.cpp);;All Files (*)"));

  if (fileName.isEmpty()) {
    return;
  }

  // Generate C++ config
  CodeGenerator generator;
  QString code = generator.generate(m_diagramEditor->fsm());

  // Write to file
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(
        this, tr("Error"),
        tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
    return;
  }

  QTextStream out(&file);
  out << code;
  file.close();

  statusBar()->showMessage(tr("Exported C++ config to: %1").arg(fileName));

  // Show success message
  QMessageBox::information(
      this, tr("Export Complete"),
      tr("C++ config has been generated successfully!\n\nFile: %1\n\nYou can "
         "now compile this code with your C++ project.")
          .arg(fileName));
}

void MainWindow::exportJson() {
  // Get FSM from diagram editor
  FSM *fsm = m_diagramEditor->fsm();

  if (!fsm || fsm->states().isEmpty()) {
    QMessageBox::warning(this, tr("No FSM to Export"),
                         tr("Please create at least one state before "
                            "exporting.\\n\\nUse 'New' to "
                            "create a project and add states to your FSM."));
    return;
  }

  // Show file save dialog
  QString selectedFilter = tr("JSON Files (*.json)");
  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Export JSON"), "", tr("JSON Files (*.json);;All Files (*)"),
      &selectedFilter);

  if (fileName.isEmpty()) {
    return;
  }

  // Ensure .json extension
  if (!fileName.endsWith(".json", Qt::CaseInsensitive)) {
    fileName += ".json";
  }

  // Use JSONSerializer to save
  JSONSerializer serializer;
  if (!serializer.save(fsm, fileName)) {
    QMessageBox::critical(this, tr("Export Failed"),
                          tr("Failed to export FSM to JSON file."));
    return;
  }

  statusBar()->showMessage(tr("Exported JSON to: %1").arg(fileName));

  // Show success message
  QMessageBox::information(this, tr("Export Complete"),
                           tr("FSM has been exported to JSON "
                              "successfully!\\n\\nFile: %1\\n\\nYou can "
                              "now use this file to import the FSM later.")
                               .arg(fileName));
}

void MainWindow::updateDiagramFromCode() {
  QString code = m_codePreviewPanel->code();
  if (code.isEmpty()) {
    return;
  }

  // Parse code
  CodeParser parser;
  FSM *newFsm = parser.parse(code, this);

  if (!newFsm) {
    QMessageBox::warning(
        this, tr("Parsing Failed"),
        tr("Could not parse code:\n%1").arg(parser.lastError()));
    return;
  }

  // Preserve FSM name if not found in code
  if (newFsm->name().isEmpty() && m_diagramEditor->fsm()) {
    newFsm->setName(m_diagramEditor->fsm()->name());
  }

  // Switch FSM
  // Note: Old FSM memory management depends on ownership policy.
  // They are children of MainWindow so they will be eventually deleted,
  // but good practice to delete old one if we replace it.
  FSM *oldFsm = m_diagramEditor->fsm();

  m_diagramEditor->setFSM(newFsm);

  // 1. Capture old positions
  QMap<QString, QPointF> oldPositions;
  QRectF bounds;
  if (m_diagramEditor->fsm()) {
    for (State *s : m_diagramEditor->fsm()->states()) {
      oldPositions.insert(s->name(), s->position());
      // Track bounds to know where to put new stuff
      if (bounds.isNull())
        bounds = QRectF(s->position(), QSizeF(10, 10));
      else
        bounds = bounds.united(QRectF(s->position(), QSizeF(100, 100)));
    }
  }

  // 2. Apply positions to new FSM
  QList<State *> states = newFsm->states();

  // Check how many we can preserve and if they are overlapping
  int preservedCount = 0;
  QSet<QString>
      distinctPositions; // Use string representation of point to check overlap
  bool hasOverlap = false;

  for (State *s : states) {
    if (oldPositions.contains(s->name())) {
      preservedCount++;
      QPointF pos = oldPositions.value(s->name());
      QString posKey = QString("%1,%2").arg(pos.x()).arg(pos.y());
      if (distinctPositions.contains(posKey)) {
        hasOverlap = true;
      }
      distinctPositions.insert(posKey);
    }
  }

  // Strategy:
  // 1. Fresh Import (preservedCount == 0) -> GRID
  // 2. Overlap Detected (Stack issue) -> GRID (Fix it!)
  // 3. Otherwise -> Preserve + Append

  bool useGridLayout = (preservedCount == 0) || hasOverlap ||
                       (states.count() > 5 && preservedCount < 2);

  if (useGridLayout) {
    // Fresh or Fix Grid Layout
    int cols = qCeil(qSqrt(states.count()));
    int spacing = 300;
    for (int i = 0; i < states.count(); ++i) {
      int row = i / cols;
      int col = i % cols;
      states[i]->setPosition(QPointF(col * spacing, row * spacing));
    }
    statusBar()->showMessage(
        tr("Diagram updated: Fresh Grid Layout applied (%1 states)")
            .arg(states.count()));
  } else {
    // Preserve + Append
    double nextX = bounds.isNull() ? 0 : bounds.right() + 300;
    double nextY = bounds.isNull() ? 0 : bounds.top();

    for (int i = 0; i < states.count(); ++i) {
      State *s = states[i];
      if (oldPositions.contains(s->name())) {
        s->setPosition(oldPositions.value(s->name()));
      } else {
        // New state found! Place it to the right
        s->setPosition(QPointF(nextX, nextY));
        nextX += 300;
      }
    }
    statusBar()->showMessage(tr("Diagram updated: Layout Preserved (%1 states)")
                                 .arg(states.count()));
  }

  // Switch FSM (DiagramEditor will handle scene rebuild safe and clean)
  m_diagramEditor->setFSM(newFsm);
  m_propertiesPanel->setFSM(newFsm); // Connect properties panel

  // Window title sync
  connect(newFsm, &FSM::nameChanged, this, [this](const QString &name) {
    setWindowTitle(QString("QtFSM Designer - %1").arg(name));
  });
  setWindowTitle(QString("QtFSM Designer - %1").arg(newFsm->name()));

  // Manual Mode: No auto-connect
  // connect(newFsm, &FSM::modified, this, &MainWindow::updateCodePreview);

  // Delete old FSM if it existed
  if (oldFsm) {
    oldFsm->deleteLater();
  }
}

void MainWindow::toggleTheme() {
  m_darkTheme = !m_darkTheme;
  m_toggleThemeAction->setChecked(m_darkTheme);
  applyTheme();
  statusBar()->showMessage(
      m_darkTheme ? tr("Dark theme enabled") : tr("Light theme enabled"), 2000);
}

void MainWindow::applyTheme() {
  if (m_darkTheme) {
    // Dark theme - VS Code inspired
    setStyleSheet(R"(
            QMainWindow {
                background-color: #1e1e1e;
            }
            QToolBar {
                background-color: #2d2d30;
                border: 1px solid #3e3e42;
                spacing: 8px;
                padding: 6px;
            }
            QToolButton {
                min-width: 80px;
                min-height: 36px;
                padding: 6px 12px;
                font-size: 11pt;
                color: #e0e0e0;
                background-color: #3e3e42;
                border: 1px solid #555555;
                border-radius: 4px;
            }
            QToolButton:hover {
                background-color: #094771;
                border-color: #007acc;
            }
            QToolButton:pressed {
                background-color: #005a9e;
            }
            QMenuBar {
                background-color: #2d2d30;
                color: #e0e0e0;
                border-bottom: 1px solid #3e3e42;
                padding: 4px;
                font-size: 11pt;
            }
            QMenuBar::item {
                padding: 6px 12px;
                background-color: transparent;
                color: #e0e0e0;
            }
            QMenuBar::item:selected {
                background-color: #094771;
                border-radius: 4px;
            }
            QMenu {
                font-size: 10pt;
                background-color: #2d2d30;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
            }
            QMenu::item {
                padding: 8px 30px 8px 20px;
            }
            QMenu::item:selected {
                background-color: #094771;
            }
            QStatusBar {
                background-color: #2d2d30;
                color: #e0e0e0;
                border-top: 1px solid #3e3e42;
                font-size: 10pt;
            }
            QDockWidget {
                color: #e0e0e0;
            }
            QDockWidget::title {
                background-color: #2d2d30;
                padding: 6px;
            }
        )");

    // Update diagram editor background for dark theme
    m_diagramEditor->setBackgroundBrush(QBrush(QColor(30, 30, 30)));

  } else {
    // Light theme
    setStyleSheet(R"(
            QMainWindow {
                background-color: #f5f5f5;
            }
            QToolBar {
                background-color: #ffffff;
                border: 1px solid #ddd;
                spacing: 8px;
                padding: 6px;
            }
            QToolButton {
                min-width: 80px;
                min-height: 36px;
                padding: 6px 12px;
                font-size: 11pt;
                background-color: #ffffff;
                border: 1px solid #ccc;
                border-radius: 4px;
            }
            QToolButton:hover {
                background-color: #e3f2fd;
                border-color: #2196f3;
            }
            QToolButton:pressed {
                background-color: #bbdefb;
            }
            QMenuBar {
                background-color: #ffffff;
                border-bottom: 1px solid #ddd;
                padding: 4px;
                font-size: 11pt;
            }
            QMenuBar::item {
                padding: 6px 12px;
                background-color: transparent;
            }
            QMenuBar::item:selected {
                background-color: #e3f2fd;
                border-radius: 4px;
            }
            QMenu {
                font-size: 10pt;
                border: 1px solid #ddd;
            }
            QMenu::item {
                padding: 8px 30px 8px 20px;
            }
            QMenu::item:selected {
                background-color: #e3f2fd;
            }
            QStatusBar {
                background-color: #ffffff;
                border-top: 1px solid #ddd;
                font-size: 10pt;
            }
        )");

    // Update diagram editor background for light theme
    m_diagramEditor->setBackgroundBrush(QBrush(QColor(250, 250, 250)));
  }
}

void MainWindow::showAbout() {
  AboutDialog dialog(this);
  dialog.exec();
}
