#pragma once

#include <QtWidgets/QWidget>
#include "ui_xplay1.h"

class XPlay1 : public QWidget
{
    Q_OBJECT

public:
    XPlay1(QWidget *parent = Q_NULLPTR);

public slots:
    void open();

private:
    Ui::XPlay1Class ui;
};
