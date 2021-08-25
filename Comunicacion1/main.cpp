#include "comunard.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ComunArd w;
    w.show();

    return a.exec();
}
