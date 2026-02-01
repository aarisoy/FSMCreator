#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class DiagramEditor;
class PropertiesPanel;
class CodePreviewPanel;
class DiagramViewModel;

/**
 * @brief The MainWindow class is the primary container for the entire
 * application.
 *
 * It acts as the central hub of interaction, managing the layout of dockable
 * panels (Properties, Code Preview) and the central Diagram Editor. It is
 * responsible for global actions such as New, Open, Save, and Export.
 *
 * @ingroup View
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  /**
   * @brief Constructs a new MainWindow.
   * @param parent The parent QWidget (default: nullptr).
   */
  explicit MainWindow(QWidget *parent = nullptr);

  /**
   * @brief Destroys the MainWindow.
   */
  ~MainWindow();

private:
  /**
   * @brief Sets up the initial UI layout.
   */
  void setupUi();

  /**
   * @brief Creates global QAction objects (New, Save, etc.).
   */
  void createActions();

  /**
   * @brief Creates the application menu bar hierarchy.
   */
  void createMenus();

  /**
   * @brief Creates toolbars for quick access to actions.
   */
  void createToolBars();

  /**
   * @brief Creates and arranges the dock widgets (Properties, Preview).
   */
  void createDockWidgets();

private slots:
  /**
   * @brief Clears the current project and starts a new one.
   */
  void newProject();

  /**
   * @brief Opens a JSON project file from disk.
   */
  void importJson();

  /**
   * @brief Imports a C++ source file to reverse-engineer an FSM.
   */
  void importCpp();

  /**
   * @brief Saves the current project to a JSON file.
   */
  void saveProject();

  /**
   * @brief Exports the current FSM to compliant C++ code.
   */
  void exportCpp();

  /**
   * @brief Exports (Save As) the project to a specific JSON file path.
   */
  void exportJson();

  /**
   * @brief Toggles between Light and Dark application themes.
   */
  void toggleTheme();

  /**
   * @brief Triggers immediate code generation for the preview panel.
   */
  void updateCodePreview();

  /**
   * @brief Rebuilds the visual diagram when the internal Model changes (e.g.
   * from parsing).
   */
  void updateDiagramFromCode();

  /**
   * @brief Shows the "About" dialog box.
   */
  void showAbout();

private:
  /**
   * @brief Applies the selected color theme (Light/Dark) to the application
   * style.
   */
  void applyTheme();

  // Central widget
  DiagramEditor *m_diagramEditor; ///< The central canvas for drawing the FSM.

  // ViewModel
  DiagramViewModel
      *m_viewModel; ///< The viewmodel mediating between View and Model.

  // Dock widgets
  PropertiesPanel
      *m_propertiesPanel; ///< Side panel for editing object properties.
  CodePreviewPanel
      *m_codePreviewPanel; ///< Side panel for viewing generated code.

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
  QAction *m_aboutAction;

  QString m_currentFile; ///< The currently open project file path.
  bool m_darkTheme;      ///< True if dark theme is currently active.
};

#endif // MAINWINDOW_H
