#include <QApplication>
#include "mainwindow.h"
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLoggingCategory::setFilterRules("qt.qpa.xcb=false");

    MainWindow w;
    w.show();

    return app.exec();
}
