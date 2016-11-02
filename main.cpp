#include "mainwindow.h"
#include <QApplication>

/*
 * This app will be a simluation of light source falling on a plane, including
 * objects hiding in the middle. The pusprose of this simulation is to find the
 * best UV systems positioning, in means of intesity uniformness and efficiency.
 */

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
