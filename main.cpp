#include "lauremotepowerwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LAURemotePowerDialog w;
    return w.exec();
}
