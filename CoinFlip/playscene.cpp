#include "playscene.h"
#include <QDebug>
#include <QString>
#include <QMenuBar>
#include <QPainter>
#include "mypushbutton.h"
#include <QTimer>
#include <QLabel>
#include <QDebug>
#include "mycoin.h"
#include "dataconfig.h"
#include <QPropertyAnimation>
#include <QSound>

//PlayScene::PlayScene(QWidget *parent) : QMainWindow(parent)
//{

//}

PlayScene::PlayScene(int level){
    QString str = QString("Enter level %1").arg(level);
    this->level = level;
    qDebug()<<str;


    this->setFixedSize(640,560);
    this->setWindowIcon(QIcon(":/img/coin.png"));
    this->setWindowTitle(QString("Coin Flip"));


    QMenuBar *bar = menuBar();
    setMenuBar(bar);
    QMenu *startMenu = bar->addMenu("开始");
    QAction *quitAction = startMenu->addAction("退出");
    connect(quitAction, &QAction::triggered, [=](){
        this->close();
    });


    MyPushButton *backBtn = new MyPushButton(":/img/back1.png",":/img/back2.png");
    QSound *backSound = new QSound(":/img/BackButtonSound.wav", this);
    backBtn->setParent(this);
    backBtn->move(this->width()-backBtn->width(),this->height()-backBtn->height());
    connect(backBtn, &MyPushButton::clicked, this, [=](){
        backSound->play();
        QTimer::singleShot(200, this, [=](){
            this->hide();
            emit this->playSceneBack();
        });
    });

    //显示当前关卡数
    QLabel *label = new QLabel;
    label->setParent(this);
    label->setGeometry(20, this->height()-100, 200, 100);
    QFont font;
    font.setFamily("华文新魏");
    font.setPointSize(20);
    font.setBold(true);
    label->setFont(font);
    QString s = QString("Level: %1").arg(this->level);
    label->setText(s);

    //准备胜利图片
    QLabel *winLabel = new QLabel(this);
    QPixmap winPix;
    winPix.load(QString(":/img/success.png"));
    winLabel->setFixedSize(winPix.width(), winPix.height());
    winLabel->setGeometry(0, 0, winPix.width(), winPix.height());  //setGeometry还是用来设置Label本身的尺寸为好
    winLabel->setPixmap(winPix);
    winLabel->move(0.5*(this->width()-winPix.width()), -winPix.height());  //move用来设置label的具体位置信息


    dataConfig config;
    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            this->gameArray[i][j] = config.mData[this->level][i][j];
        }
    }

    this->isWin = false;
    QSound *flipSound = new QSound(":/img/ConFlipSound.wav", this);
    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            //金币（银币）所在的背景就不叠了，感觉效果不好
//            QLabel *label = new QLabel;
//            QPixmap pix;
//            QString str = QString(":/img/coinbackground.png");
//            pix.load(str);
//            qDebug()<<"coinbackground.png w/h"<<pix.width()<<pix.height();
//            label->setParent(this);
//            label->setGeometry(0,0,pix.width(),pix.height());
//            label->setPixmap(pix);
//            label->move(190+i*70, 140+j*70);
            QString strs;
            if(this->gameArray[i][j] == 1){
                strs = QString(":/img/Coin0001.png");
            }
            else{
                strs = QString(":/img/Coin0008.png");
            }
            MyCoin *coin = new MyCoin(strs);
            coin->setParent(this);
            coin->move(190+i*70, 140+j*70);

            coin->flag = this->gameArray[i][j];
            coin->active = true;
            coin->posX = i;
            coin->posY = j;
            this->coinBtn[i][j] = coin;

            connect(coin, &MyCoin::clicked, [=](){
                if(!coin->protect){
                    flipSound->play();
                    coin->timer->start(30);
                    coin->protect = true;
                    this->gameArray[i][j] = this->gameArray[i][j] == 0 ? 1 : 0;  //最好放到changeFlag完成后进行

                    QTimer::singleShot(250, this, [=](){
                        //翻转上面
                        if(coin->posY - 1 >= 0){
                            if(!coinBtn[coin->posX][coin->posY - 1]->protect){
                                qDebug()<<"flip up";
                                coinBtn[coin->posX][coin->posY - 1]->timer->start(30);
                                coinBtn[coin->posX][coin->posY - 1]->protect = true;
                                this->gameArray[coin->posX][coin->posY - 1] = this->gameArray[coin->posX][coin->posY - 1] == 0 ? 1 : 0;
                            }
                        }
                        //翻转下面
                        if(coin->posY + 1 <= 3){
                            if(!coinBtn[coin->posX][coin->posY + 1]->protect){
                                qDebug()<<"flip down";
                                coinBtn[coin->posX][coin->posY + 1]->timer->start(30);
                                coinBtn[coin->posX][coin->posY + 1]->protect = true;
                                this->gameArray[coin->posX][coin->posY + 1] = this->gameArray[coin->posX][coin->posY + 1] == 0 ? 1 : 0;
                            }
                        }
                        //翻转左侧
                        if(coin->posX - 1 >= 0){
                            if(!coinBtn[coin->posX - 1][coin->posY]->protect){
                                qDebug()<<"flip left";
                                coinBtn[coin->posX - 1][coin->posY]->timer->start(30);
                                coinBtn[coin->posX - 1][coin->posY]->protect = true;
                                this->gameArray[coin->posX - 1][coin->posY] = this->gameArray[coin->posX - 1][coin->posY] == 0 ? 1 : 0;
                            }
                        }
                        //翻转右侧
                        if(coin->posX + 1 <= 3){
                            if(!coinBtn[coin->posX + 1][coin->posY]->protect){
                                qDebug()<<"flip right";
                                coinBtn[coin->posX + 1][coin->posY]->timer->start(30);
                                coinBtn[coin->posX + 1][coin->posY]->protect = true;
                                this->gameArray[coin->posX + 1][coin->posY] = this->gameArray[coin->posX + 1][coin->posY] == 0 ? 1 : 0;
                            }
                        }

                        //判断游戏是否胜利
                        bool temp = true;
                        for(int k=0; k<4; k++){
                            for(int s=0; s<4; s++){
                                temp &= (this->gameArray[k][s]==1?1:0);
                            }
                        }
                        if(temp){
                            this->isWin = temp;
                            for(int k=0; k<4; k++){
                                for(int s=0; s<4; s++){
                                    coinBtn[k][s]->active = false;
                                }
                            }
                            qDebug()<<"SUCCESS";

                            //动态显示胜利图片
                            //创建动画对象
                            QPropertyAnimation *animation = new QPropertyAnimation(winLabel, "geometry");
                            //设置动画间隔
                            animation->setDuration(1000);
                            //设置动画曲线
                            animation->setEasingCurve(QEasingCurve::OutBounce);
                            //设置起始位置
                            animation->setStartValue(QRect(winLabel->x(), winLabel->y(), winLabel->width(), winLabel->height()));
                            //设置结束位置
                            animation->setEndValue(QRect(winLabel->x(), winLabel->y()+140, winLabel->width(), winLabel->height()));
                            //启动动画
                            animation->start();
                        }
                    });
                }
            });
        }
    }
}

//重新实现paintEvent事件画背景图
void PlayScene::paintEvent(QPaintEvent *){
    QPainter painter(this);
    QPixmap pix;

    pix.load(":/img/background.jpg");
    painter.drawPixmap(0,0,this->width(),this->height(),pix);
}
