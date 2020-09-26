#include "xplay1.h"
#include <QtWidgets/QApplication>
#include "XFFmpeg.h"
#include <iostream>
using std::cout;
using std::endl;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AVPacket pkt;
    const char* path = "1024.mp4";

    int re = XFFmpeg::get()->open(path);
    if (0 != re) {
        cout << "open " << path << " failed: " << XFFmpeg::get()->get_error(re) << endl;
        return -1;
    }
    else {
        cout << "open " << path << " success! The total length is " << XFFmpeg::get() ->get_duration_ms() << " ms." << endl;
    }

    while (1) {
        pkt = XFFmpeg::get()->read();
        if (pkt.size == 0)break;
        cout << " - PTS = " << pkt.pts << endl;
        av_packet_unref(&pkt);
    }

    XFFmpeg::get()->close();
    XPlay1 w;
    w.show();
    return a.exec();
}