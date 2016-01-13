/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

#include <QApplication>

#include "cmainwindow.h"


int main (int argc, char *argv[])
{
    QApplication a (argc, argv);
    CMainWindow w;
    w.show();

    return a.exec();
}
