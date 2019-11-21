#include "mainwindow.h"
#include "logutils.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LOGUTILS::initLogging();

    MainWindow w;
    w.show();
    return a.exec();
}
