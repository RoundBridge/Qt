#include <QDebug>
#include <QIODevice>
#include "operate.h"
#include "ui_operate.h"


Operate::Operate(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Operate)
{
    ui->setupUi(this);
    // mCmd = 0;
    // mSeq = 1;
    // mReceiverPort = 6666;
    // mReceiverAddress.setAddress("10.168.1.171");
    // mSocket = new QUdpSocket(this);
    // mQueryTimer = new QTimer(this);
    // connect(mQueryTimer, &QTimer::timeout, this, &Operate::doQuery);
    // mQueryTimer->setSingleShot(false);
    // mQueryTimer->start(15000);

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
    // if (mSocket) {
    //     mSocket->close();
    //     delete mSocket;
    //     mSocket = nullptr;
    // }
}

void Operate::setController(Controller* ctrl) {
    mCtrl = ctrl;
}

// void Operate::doQuery() {
//     QJsonObject e;
//     QByteArray b;
//     makeCmdAndSend(CMD_MAIN_ACTUATOR_QUERY, QD_MESSAGE_TYPE_JSON, e, b);
//     // qDebug() << "query executed";
//     mQueryTimer->setInterval(500);
// }

void Operate::on_reset_clicked()
{
    qDebug() << "reset clicked";
    mCtrl->dealCmd(CTRL_RESET);
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
    mCtrl->dealCmd(CTRL_STOP);
}

void Operate::on_recover_clicked()
{
    qDebug() << "recover clicked";
    mCtrl->dealCmd(CTRL_RECOVER);
}

void Operate::on_pause_clicked()
{
    qDebug() << "pause clicked";
    mCtrl->dealCmd(CTRL_PAUSE);
}

void Operate::on_resume_clicked()
{
    qDebug() << "resume clicked";
    mCtrl->dealCmd(CTRL_RESUME);
}

