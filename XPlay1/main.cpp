#include "xplay1.h"
#include <QtWidgets/QApplication>
#include "XFFmpeg.h"
#include <iostream>
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    //没有下面这句话，播放界面一直显示黑屏，具体原因不清楚，下面是摘自网上的一段解释：
    /*
        问题所在，程序直接去调用了OpenGL ES库，而在OpenGL ES库中是不支持glBegin()等函数的，
        所以我们需要让程序去连接到OpenGL库，这其实也很简单，在mian.cpp调用QApplication之前
        设置使用OpenGL库的属性
    */
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication a(argc, argv);
    XPlay1 w;
    w.show();
    return a.exec();
}