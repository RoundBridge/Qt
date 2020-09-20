#include "xplay1.h"
#include <QtWidgets/QApplication>
#include "XFFmpeg.h"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (true != XFFmpeg::get()->open("1024.mp4")) {
        return -1;
    }
    XPlay1 w;
    w.show();
    return a.exec();
}
