#include "VideoPlay.h"
#include <QtWidgets/QApplication>
#include <libavcodec/avcodec.h>
#include "XDemux.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	XDemux testDemux;
	testDemux.Open("test.mp4");
    VideoPlay w;
    w.show();
    return a.exec();
}
