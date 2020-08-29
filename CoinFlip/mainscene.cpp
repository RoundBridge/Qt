#include "mainscene.h"
#include "ui_mainscene.h"
#include <QPainter>
#include <QDebug>
#include "mypushbutton.h"
#include <QTimer>
#include <QSound>

MainScene::MainScene(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainScene)
{
    ui->setupUi(this);
    //配置主场景
    //设置固定宽高
    this->setFixedSize(640,560);
    //设置图标
    this->setWindowIcon(QIcon(":/img/coin.png"));
    //设置标题
    this->setWindowTitle("Coin Flip");
    //设置退出
    connect(ui->actionquit, &QAction::triggered, [=](){
        this->close();
    });

    //开始按钮
    MyPushButton *startBtn = new MyPushButton(":/img/start.png");
    startBtn->setParent(this);
    startBtn->move(this->width()*0.5-startBtn->width()*0.5, this->height()*0.7);
    QSound *startSound = new QSound(":/img/TapButtonSound.wav", this);

    chooseScene = new ChooseLevelScene;
    //chooseScene->setAttribute(Qt::WA_DeleteOnClose); //该函数和~MainScene()中的“delete chooseScene;”只能选其一，否则会重复释放

    connect(startBtn, &QPushButton::clicked,[=](){
        startSound->play();
        startBtn->zoom(0);
        startBtn->zoom(1);

        //点击开始按钮进入到ChooseLevelScene中
        //延时进入
        QTimer::singleShot(400, this, [=](){ //0.5秒后让当前对象this执行lambda式中的操作
            //隐藏自身
            this->hide();
            //将选择关卡界面位置设置成与主界面一致
            chooseScene->setGeometry(this->geometry());
            //显示选择关卡界面
            chooseScene->show();
        });
    });

    connect(chooseScene, &ChooseLevelScene::chooseSceneBack, this, [=](){
        //将主界面位置设置成与选择关卡界面位置一致
        this->setGeometry(chooseScene->geometry());
        this->show();
    });

}


void MainScene::paintEvent(QPaintEvent *){
    QPainter painter(this);
    QPixmap pix;
    //画背景图标
    pix.load(":/img/background.jpg");
    painter.drawPixmap(0,0,this->width(), this->height(), pix);
    //画title图标
    pix.load(":/img/title.png");
    pix = pix.scaled(pix.width()*0.4, pix.height()*0.4);
    painter.drawPixmap(10,30,pix);
}

MainScene::~MainScene()
{
    delete ui;
    delete chooseScene;
}

