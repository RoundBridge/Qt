#ifndef PLAYSCENE_H
#define PLAYSCENE_H

#include <QMainWindow>
#include "mycoin.h"

class PlayScene : public QMainWindow
{
    Q_OBJECT
public:
    //explicit PlayScene(QWidget *parent = nullptr);
    PlayScene(int level);
    //重新实现paintEvent事件画背景图
    void paintEvent(QPaintEvent *);

    int gameArray[4][4];  //维护每个关卡金币和银币的具体位置数据
    MyCoin *coinBtn[4][4];  //保存每个coin对象的地址

signals:
    void playSceneBack();  //点击返回发出的信号
private:
    int level;  //游戏关卡
    bool isWin = false;  //游戏是否胜利
};

#endif // PLAYSCENE_H
