#include <QDebug>
#include <QIODevice>
#include "operate_joint.h"
#include "mainwindow.h"
#include "ui_operate_joint.h"

operate_joint::operate_joint(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::operate_joint)
{
    mParent = parent;
    ui->setupUi(this);
    ui->stop->setStyleSheet(
        "QPushButton {"
        "   background-color: rgb(220, 0, 0);"
        "   color: white;"              // 白色文字
        "   border: 2px solid #8B4513;" // 棕色边框
        "}"
        "QPushButton:hover {"
        "   background-color: rgb(255, 0, 0);" // 悬停时
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(200, 0, 0);"
        "    border-style: inset;"
        "}"
        );

    ui->pause->setStyleSheet(
        "QPushButton {"
        "   background-color: rgb(220, 100, 0);"
        "   color: white;"              // 白色文字
        "   border: 2px solid #8B4513;" // 棕色边框
        "}"
        "QPushButton:hover {"
        "   background-color: rgb(255, 100, 0);" // 悬停时
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(200, 100, 0);"
        "    border-style: inset;"
        "}"
        );
}

operate_joint::~operate_joint()
{
    delete ui;
}

void operate_joint::setController(Controller* ctrl) {
    mCtrl = ctrl;
}

void operate_joint::on_back_clicked()
{
    MainWindow* p = dynamic_cast<MainWindow*>(mParent);
    hide();
    p->getOperateInstance()->show();
}

