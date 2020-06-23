#include "VideoPlay.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoPlay w;
    w.show();
    return a.exec();
}
