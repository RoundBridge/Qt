#include "chooselevelscene.h"
#include "playscene.h"
#include <QMenuBar>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QFont>
#include <QSound>

ChooseLevelScene::ChooseLevelScene(QWidget *parent)
    : QMainWindow(parent)
{
    this->setFixedSize(640,560);
    this->setWindowIcon(QIcon(":/img/coin.png"));
    this->setWindowTitle("选择关卡");

    //利用label显示动图来设置标签
    QLabel *label = new QLabel;
    QMovie *movie = new QMovie(":/img/coinflip3.gif");
    movie->setScaledSize(QSize(100,100));
    label->setParent(this);
    label->setGeometry(this->width()-98, 20, 100, 100);
    label->setMovie(movie);
    movie->start();
    label->show();

    //创建菜单栏
    QMenuBar *bar = menuBar();
    setMenuBar(bar);
    //创建开始菜单
    QMenu *startMenu = bar->addMenu("开始");
    //创建退出菜单项
    QAction *quitAction = startMenu->addAction("退出");
    //点击退出
    connect(quitAction, &QAction::triggered, this, [=](){
        this->close();
    });

    //设置返回按钮
    QSound *backSound = new QSound(":/img/BackButtonSound.wav", this);
    //back1.png和back2.png图像尺寸是一样大的，都为128*128,但是back1.png的图标部分大一点，
    //back2.png的背景部分大一点，这样点击时的效果好一点，两张图片的中心点在一起，否则点击时有一种移位感
    MyPushButton *backBtn = new MyPushButton(":/img/back1.png",":/img/back2.png");
    backBtn->setParent(this);
    qDebug()<<"before this size w/h"<<this->width()<<this->height();
    backBtn->move(this->width()-backBtn->width(),this->height()-backBtn->height());
    qDebug()<<"backBtn size w/h"<<backBtn->width()<<backBtn->height();
    //点击返回
    connect(backBtn, &MyPushButton::clicked, [=](){
        qDebug()<<"Back!";
        backSound->play();
        QTimer::singleShot(200, this, [=](){
            this->hide();
            emit this->chooseSceneBack();
        });
    });

    //创建选择关卡按钮
    for(int i=0; i<20; i++){
        MyPushButton *menuBtn = new MyPushButton(":/img/level.png");
        menuBtn->setParent(this);
        menuBtn->move(128 + i%4 * 96, 100 + i/4 * 72);

        QLabel *label = new QLabel;
        label->setParent(this);
        label->setFixedSize(menuBtn->width(), menuBtn->height());
        label->setText(QString::number(1+i));
        label->setFont(QFont("Microsoft YaHei", 18, 100));
        label->move(128 + i%4 * 96, 100 + i/4 * 72);
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        //设置鼠标点击事件穿透属性
        label->setAttribute(Qt::WA_TransparentForMouseEvents);

        //监听每个按钮的点击事件
        connect(menuBtn, &MyPushButton::clicked, [=](){
            QString str = QString("Select %1 level!").arg(i+1);
            qDebug()<<str;

            //进入游戏界面场景
            this->play = new PlayScene(i+1);
            //将游戏界面场景设置成和选择关卡场景一致
            play->setGeometry(this->geometry());
            play->show();
            this->hide();
            connect(play, &PlayScene::playSceneBack, this, [=](){
                //将返回到选择关卡按钮的位置设置成与游戏界面位置一致
                this->setGeometry(play->geometry());
                delete this->play;
                this->play = NULL;
                this->show();
            });
        });

    }
}

void ChooseLevelScene::paintEvent(QPaintEvent *){
    QPainter painter(this);
    QPixmap pix;
    //加载背景
    pix.load(":/img/background.jpg");
    painter.drawPixmap(0,0,this->width(), this->height(), pix);

}

ChooseLevelScene::~ChooseLevelScene(){
    delete label;
    delete movie;
    qDebug()<<"Deconstructor of ChooseLevelScene";
}









