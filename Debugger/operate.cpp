#include <QDebug>
#include <QIODevice>
#include "operate.h"
#include "mainwindow.h"
#include "ui_operate.h"


Operate::Operate(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Operate)
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

Operate::~Operate()
{
    delete ui;
}

void Operate::setController(Controller* ctrl) {
    mCtrl = ctrl;
}

void Operate::on_reset_clicked()
{
    qDebug() << "reset clicked";
    mCtrl->dealCmd(CTRL_RESET, MAIN_ACTUATOR_NAME);
}

void Operate::on_contentIP_editingFinished()
{
    qDebug() << "target IP:" << ui->contentIP->text();
}

void Operate::on_contentPort_editingFinished()
{
    bool ok;
    int port = ui->contentPort->text().toInt(&ok);

    if (!ok) {
        qDebug() << "target port:" << ui->contentPort->text() << " error";
    } else {
        qDebug() << "target port:" << port;
    }
}

void Operate::on_stop_clicked()
{
    qDebug() << "stop clicked";
    mCtrl->dealCmd(CTRL_STOP, MAIN_ACTUATOR_NAME);
}

void Operate::on_recover_clicked()
{
    qDebug() << "recover clicked";
    mCtrl->dealCmd(CTRL_RECOVER, MAIN_ACTUATOR_NAME);
}

void Operate::on_pause_clicked()
{
    qDebug() << "pause clicked";
    mCtrl->dealCmd(CTRL_PAUSE, MAIN_ACTUATOR_NAME);
}

void Operate::on_resume_clicked()
{
    qDebug() << "resume clicked";
    mCtrl->dealCmd(CTRL_RESUME, MAIN_ACTUATOR_NAME);
}

void Operate::on_prepareStrip_clicked()
{
    qDebug() << "prepare strip clicked";
    mCtrl->dealCmd(CTRL_PREPARE_STRIP, MAIN_ACTUATOR_NAME);
}

void Operate::on_ActuatorState_clicked()
{
    MainWindow* p = dynamic_cast<MainWindow*>(mParent);
    hide();
    p->getStateInstance()->show();
}

void Operate::on_strip_clicked()
{
    qDebug() << "strip clicked";
    mCtrl->dealCmd(CTRL_STRIP, MAIN_ACTUATOR_NAME);
}

void Operate::on_continueStrip_clicked(bool checked)
{
    qDebug() << "continue strip " << checked;
    mCtrl->setParam(MAIN_ACTUATOR_NAME, CONTINUE_STRIP, &checked, sizeof(checked));
}

void Operate::on_joint_oprate_clicked()
{
    MainWindow* p = dynamic_cast<MainWindow*>(mParent);
    hide();
    p->getJointOperateInstance()->show();
}

