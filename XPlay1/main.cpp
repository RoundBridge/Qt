#include "xplay1.h"
#include <QtWidgets/QApplication>
#include "XFFmpeg.h"
#include <iostream>
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    //û��������仰�����Ž���һֱ��ʾ����������ԭ�������������ժ�����ϵ�һ�ν��ͣ�
    /*
        �������ڣ�����ֱ��ȥ������OpenGL ES�⣬����OpenGL ES�����ǲ�֧��glBegin()�Ⱥ����ģ�
        ����������Ҫ�ó���ȥ���ӵ�OpenGL�⣬����ʵҲ�ܼ򵥣���mian.cpp����QApplication֮ǰ
        ����ʹ��OpenGL�������
    */
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication a(argc, argv);
    XPlay1 w;
    w.show();
    return a.exec();
}