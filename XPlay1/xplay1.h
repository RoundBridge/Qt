#pragma once

#include <QtWidgets/QWidget>
#include "ui_xplay1.h"

class XPlay1 : public QWidget
{
    Q_OBJECT

public:
    XPlay1(QWidget *parent = Q_NULLPTR);
    void timerEvent(QTimerEvent* e);
    void resizeEvent(QResizeEvent* e);
    int get_play_state();
    void set_play_state(int);

public slots:
    void open();
    void sliderPress();
    void sliderRelease();
    void play();

private:
    Ui::XPlay1Class ui;
    int playState = 0;  // ��ʾ��ǰ��������״̬��0 - ֹͣ��1 - ���ţ�2 - ��ͣ
};
