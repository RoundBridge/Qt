#ifndef CHOOSELEVELSCENE_H
#define CHOOSELEVELSCENE_H

#include <QMainWindow>
#include <QMovie>
#include <QLabel>
#include "mypushbutton.h"
#include "playscene.h"

class ChooseLevelScene : public QMainWindow
{
    Q_OBJECT
public:
    explicit ChooseLevelScene(QWidget *parent = nullptr);
    ~ChooseLevelScene();
    //重新实现paintEvent事件画背景图
    void paintEvent(QPaintEvent *);
signals:
    void chooseSceneBack();  //点击返回发出的信号
private:
    QLabel *label = NULL;
    QMovie *movie = NULL;
    PlayScene *play = NULL;  //游戏场景指针
};

#endif // CHOOSELEVELSCENE_H
