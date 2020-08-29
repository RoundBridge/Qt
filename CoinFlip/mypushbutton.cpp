#include "mypushbutton.h"
#include <QDebug>
#include <QPropertyAnimation>
//MyPushButton::MyPushButton(QWidget *parent) : QPushButton(parent)
//{

//}

MyPushButton::MyPushButton(QString normalImg, QString pressImg){
    this->normalImg = normalImg;
    this->pressImg = pressImg;

    QPixmap pix;
    bool ret = pix.load(normalImg);
    if(!ret){
        qDebug()<<"normalImg load failed!";
        return;
    }
    //qDebug()<<"normalImg size w/h"<<pix.width()<<pix.height();
    //qDebug()<<"before this size w/h"<<this->width()<<this->height();

    //设置图片固定大小
    this->setFixedSize(pix.width(), pix.height());
    //qDebug()<<"after this size w/h"<<this->width()<<this->height();

    //设置不规则图片样式
    this->setStyleSheet("QPushButton{border:0px}");

    //设置图标
    this->setIcon(pix);

    //设置图标大小
    //qDebug()<<"1-IconSize w/h"<<this->iconSize().rwidth()<<this->iconSize().rheight();
    this->setIconSize(QSize(pix.width()*0.5, pix.height()*0.5));
    //qDebug()<<"2-IconSize w/h"<<this->iconSize().rwidth()<<this->iconSize().rheight();
}

void MyPushButton::zoom(int flag){
    //创建动画对象
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    //设置动画时间间隔
    animation->setDuration(200);
    //设置起始位置
    animation->setStartValue(QRect(this->x(),this->y()+flag*4,this->width(),this->height()));
    //设置结束位置
    animation->setEndValue(QRect(this->x(),this->y()+(1-flag)*4,this->width(),this->height()));
    //设置动画曲线
    animation->setEasingCurve(QEasingCurve::OutBounce);
    //启动动画效果
    animation->start();
}

void MyPushButton::mousePressEvent(QMouseEvent *e)
{
    if(this->pressImg != ""){
        QPixmap pix;
        bool ret = pix.load(this->pressImg);
        if(!ret){
            qDebug()<<"pressImg load failed!";
            return;
        }

        //设置图片固定大小
        this->setFixedSize(pix.width(), pix.height());

        //设置不规则图片样式
        this->setStyleSheet("QPushButton{border:0px}");

        //设置图标
        this->setIcon(pix);

        //设置图标大小
        this->setIconSize(QSize(pix.width()*0.5, pix.height()*0.5));
    }
    //让父类执行剩余的其他操作
    return QPushButton::mousePressEvent(e);
}

void MyPushButton::mouseReleaseEvent(QMouseEvent *e)
{
    if(this->pressImg != ""){
        QPixmap pix;
        bool ret = pix.load(this->normalImg);
        if(!ret){
            qDebug()<<"normalImg load failed!";
            return;
        }

        //设置图片固定大小
        this->setFixedSize(pix.width(), pix.height());

        //设置不规则图片样式
        this->setStyleSheet("QPushButton{border:0px}");

        //设置图标
        this->setIcon(pix);

        //设置图标大小
        this->setIconSize(QSize(pix.width()*0.5, pix.height()*0.5));
    }
    //让父类执行剩余的其他操作
    return QPushButton::mouseReleaseEvent(e);
}
