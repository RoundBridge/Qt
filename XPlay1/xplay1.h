#pragma once

#include <QtWidgets/QWidget>
#include "ui_xplay1.h"

class XPlay1 : public QWidget
{
    Q_OBJECT

public:
    XPlay1(QWidget *parent = Q_NULLPTR);
    void timerEvent(QTimerEvent* e);

public slots:
    void open();
    void sliderPress();
    void sliderRelease();

private:
    Ui::XPlay1Class ui;
};
