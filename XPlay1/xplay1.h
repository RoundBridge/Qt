#pragma once

#include <QReadWriteLock>
#include <QtWidgets/QWidget>
#include "ui_xplay1.h"

class XPlay1 : public QWidget
{
    Q_OBJECT

public:
    XPlay1(QWidget *parent = Q_NULLPTR);
    void timerEvent(QTimerEvent* e);
    void resizeEvent(QResizeEvent* e);
    void closeEvent(QCloseEvent* e);
    int get_play_state();
    void set_play_state(int);
    static void rlock();
    static void wlock();
    static void unlock();
    static QReadWriteLock rwlock;
    static bool bSeek;

public slots:
    void open();
    void sliderPress();
    void sliderRelease();
    void play();

private:
    Ui::XPlay1Class ui;
    int playState = 0;  // 表示当前播放器的状态：0 - 停止，1 - 播放，2 - 暂停
};
