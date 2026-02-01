#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class DiagramEditor;
class PropertiesPanel;
class CodePreviewPanel;
class DiagramViewModel;

/**
 * @brief The MainWindow class - Main application window
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  void setupUi();
  void createActions();
  void createMenus();
  void createToolBars();
  void createDockWidgets();

private slots:
  void newProject();
  void importJson();
  void importCpp();
  void saveProject();
  void exportCpp();
  void exportJson();
  void toggleTheme();
  void updateCodePreview();
  void updateDiagramFromCode();

private:
  void applyTheme();

  // Central widget
  DiagramEditor *m_diagramEditor;

  // ViewModel
  DiagramViewModel *m_viewModel;

  // Dock widgets
  PropertiesPanel *m_propertiesPanel;
  CodePreviewPanel *m_codePreviewPanel;

  // Actions
  QAction *m_newAction;
  QAction *m_importJsonAction;
  QAction *m_importCppAction;
  QAction *m_saveAction;
  QAction *m_exportAction;
  QAction *m_exportJsonAction;
  QAction *m_exitAction;
  QAction *m_undoAction;
  QAction *m_redoAction;
  QAction *m_toggleThemeAction;

  QString m_currentFile;
  bool m_darkTheme;
};

#endif // MAINWINDOW_H
