#ifndef MYCOIN_H
#define MYCOIN_H

#include <QPushButton>
#include <QTimer>
#include <QMouseEvent>

class MyCoin : public QPushButton
{
    Q_OBJECT
public:
    //explicit MyCoin(QWidget *parent = nullptr);
    MyCoin(QString btnImg);  //btnImg用于指明是金币图片还是银币图片
    void mousePressEvent(QMouseEvent *e);
    void changeFlag();

    QTimer *timer;
    int posX, posY;
    int min = 1;
    int max = 8;
    bool flag;  //1:当前是金币  0:当前是银币
    bool active = true;  //1:表示当前处于活动状态，响应鼠标点击  0:表示当前处于非活动状态，不响应鼠标点击
    bool protect = false;  //1:表示处于保护状态  0:表示处于非保护状态

signals:
};

#endif // MYCOIN_H
