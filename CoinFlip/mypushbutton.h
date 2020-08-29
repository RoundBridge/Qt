#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QPushButton>
#include <QMouseEvent>
class MyPushButton : public QPushButton
{
    Q_OBJECT
public:
    //explicit MyPushButton(QWidget *parent = nullptr);
    //构造函数 参数1：正常显示的图片路径  参数2：按下后显示的图片路径
    MyPushButton(QString normalImg, QString pressImg="");
    void zoom(int flag); //flag: 0 向下跳动 1 向上跳动
    //重写按钮按下和释放事件
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private:
    QString normalImg;
    QString pressImg;


signals:

};

#endif // MYPUSHBUTTON_H
