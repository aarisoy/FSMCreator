#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class DiagramEditor;
class PropertiesPanel;
class CodePreviewPanel;

/**
 * @brief The MainWindow class - Main application window
 */
class MainWindow : public QMainWindow
{
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
    void openProject();
    void saveProject();
    void exportCpp();
    void toggleTheme();
    void updateCodePreview();
    void updateDiagramFromCode();

private:
    void applyTheme();
    
    // Central widget
    DiagramEditor *m_diagramEditor;
    
    // Dock widgets
    PropertiesPanel *m_propertiesPanel;
    CodePreviewPanel *m_codePreviewPanel;
    
    // Actions
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_exportAction;
    QAction *m_exitAction;
    QAction *m_toggleThemeAction;
    
    QString m_currentFile;
    bool m_darkTheme;
};

#endif // MAINWINDOW_H
