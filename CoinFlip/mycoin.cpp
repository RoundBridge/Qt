#include "mycoin.h"
#include <QPixmap>
#include <QDebug>

//MyCoin::MyCoin(QWidget *parent) : QPushButton(parent)
//{

//}

MyCoin::MyCoin(QString btnImg){
    QPixmap pix;
    bool ret;

    ret = pix.load(btnImg);
    if(!ret){
        QString s;
        s = QString("%1 load FAILED!").arg(btnImg);
        qDebug()<<s;
        return;
    }

    this->setFixedSize(pix.width(),pix.height());
    this->setStyleSheet(QString("QPushButton{border:0px;}"));
    this->setIcon(pix);
    this->setIconSize(QSize(pix.width(),pix.height()));

    this->timer = new QTimer(this);
    connect(this->timer, &QTimer::timeout, [=](){
        QPixmap pix;
        QString s;
        bool ret;

        if(this->flag){
            s = QString(":/img/Coin000%1.png").arg(this->min++);
        }else{
            s = QString(":/img/Coin000%1.png").arg(this->max--);
        }

        ret = pix.load(s);
        if(!ret){
            qDebug()<<s;
            return;
        }
        this->setFixedSize(pix.width(),pix.height());
        this->setStyleSheet(QString("QPushButton{border:0px;}"));
        this->setIcon(pix);
        this->setIconSize(QSize(pix.width(),pix.height()));

        if(this->min > this->max){
            this->min = 1;
            this->max = 8;
            this->changeFlag();
        }
    });
}

void MyCoin::changeFlag(){
    if(this->flag){
        this->flag = false;
    }else{
        this->flag = true;
    }
    timer->stop();
    this->protect = false;
}

void MyCoin::mousePressEvent(QMouseEvent *e){
    if(!this->active){
        //完成当前关卡翻金币任务
    }else{
        QPushButton::mousePressEvent(e);
    }
}
