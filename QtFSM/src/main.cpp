#include <QApplication>
#include "view/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application metadata
    QApplication::setOrganizationName("QtFSM");
    QApplication::setApplicationName("QtFSM Designer");
    QApplication::setApplicationVersion("1.0.0");
    
    // Create and show main window
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}
