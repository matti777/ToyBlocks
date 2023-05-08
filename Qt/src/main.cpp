#include "mainwindow.h"

#include <QtGui/QApplication>
#include "GLWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

//    MainWindow mainWindow;
//    mainWindow.setOrientation(MainWindow::ScreenOrientationAuto);
//#ifdef __BUILD_DESKTOP__
//    mainWindow.resize(800, 600);
//#endif
//    mainWindow.showExpanded();

    GLWidget widget(NULL);

#ifndef __BUILD_DESKTOP__
    widget.showFullScreen();
#else
    widget.resize(854, 480);
    //widget.resize(480, 480); // For taking square screenshots
    widget.show();
#endif

    return app.exec();
}
