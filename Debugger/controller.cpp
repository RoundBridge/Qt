#include <QDebug>
#include "crc.h"
#include "controller.h"
#include "mainwindow.h"

Controller::Controller(QObject *parent)
    : QObject{parent}
{
    mCtrlCmd = 0;
    mIsStop = mIsPause = false;
    mWin = dynamic_cast<MainWindow*>(parent);
    memset(mLinkSet, 0, sizeof(mLinkSet));

    for (int i = Link_UDP; i < Link_num; ++i) {
        mLinkSet[i] = Link::createLink(i, 0);
    }

    mEndSet[MAIN_ACTUATOR_NAME] = mEndCreator.create(MAIN_ACTUATOR_NAME, this, mLinkSet[Link_UDP]);
    mEndSet[JOINT_NAME] = mEndCreator.create(JOINT_NAME, this, mLinkSet[Link_UDP]);
    mEndSet[ATTITUDE_NAME] = mEndCreator.create(ATTITUDE_NAME, this, mLinkSet[Link_UDP]);

    mElapsedTimer.start();

    mPoller = new std::thread(&Controller::poll, this);
    mPoller->detach();

    connect(&mQueryTimer, &QTimer::timeout, this, &Controller::query);
    mQueryTimer.setSingleShot(false);
    mQueryTimer.start(15000);
}

bool Controller::setParam(const std::string& end, uint32_t key, void* data, uint32_t dataLen) {
    if (mEndSet[end] && data && dataLen > 0) {
        return mEndSet[end]->setParam(key, data, dataLen);
    }
    return false;
}

bool Controller::getParam(const std::string& end, uint32_t key, void* data, uint32_t dataLen) {
    if (mEndSet[end] && data && dataLen > 0) {
        return mEndSet[end]->getParam(key, data, dataLen);
    }
    return false;
}

bool Controller::dealCmd(uint32_t cmd, const std::string& end) {
    mIsStop = cmd == CTRL_STOP ? true : false;
    mIsPause = cmd == CTRL_PAUSE ? true : false;
    mCtrlCmd = cmd;

    auto it = mEndSet.find(end);
    if (it != mEndSet.end()) {
        return mEndSet[end]->execute(cmd);
    } else {
        qDebug() << "End " << end << " not exist";
        return false;
    }
}

bool Controller::query() {
    qint64 tick = getElapsedTimeMs();

    for (auto it = mEndSet.begin(); it != mEndSet.end(); ++it) {
        it->second->updateEndConnectState(false, tick); //检测是否断连
        it->second->query();
    }

    mQueryTimer.setInterval(500);
    return true;
}

void Controller::poll() {
    QUdpSocket udpSocket;

    if(!udpSocket.bind(QHostAddress::Any, CENTER_PORT)) {
        qDebug() << "Udp bind failed!";
        return;
    }

    while (1) {
        // 阻塞等待数据到达（超时时间3000ms）
        if (udpSocket.waitForReadyRead(3000)) {
            while (udpSocket.hasPendingDatagrams()) {
                QByteArray datagram;
                datagram.resize(udpSocket.pendingDatagramSize());
                udpSocket.readDatagram(datagram.data(), datagram.size());
                qDebug() << "Received: " << datagram;
                analyseData(datagram);
            }
        }
    }
}

void Controller::analyseData(QByteArray &data) {
    uint32_t data_length;
    unsigned int crc_calc;
    data_length = data.size();

    QByteArray crc_body = data.mid(8, data_length - 8);
    crc_calc = getCRC32((unsigned char *)(crc_body.data()), data_length - 8);

    QByteArray msg_head = data.left(sizeof(QDMessageHdr));
    QByteArray msg_body = data.mid(sizeof(QDMessageHdr), data_length - sizeof(QDMessageHdr));
    QDMessageHdr *hdr = (QDMessageHdr *)msg_head.data();

    if (hdr->magic != 0x4E5A4451) {
        qDebug() << "magic " << hdr->magic << " error!\n";
        return;
    }

    if (hdr->len != data_length || hdr->len <= sizeof(QDMessageHdr)) {
        qDebug() << "hdr->len " << hdr->len << " != length " << data_length;
        return;
    }

    if (hdr->crc32 != crc_calc) {
        qDebug() << "CRC dismatch, recv crc is " << hdr->crc32 << " calc crc is " << crc_calc;
        return;
    }

    if (hdr->type == QD_MESSAGE_TYPE_JSON) {
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(msg_body, &error);
        if (error.error == QJsonParseError::NoError) {
            if (jsonDoc.isObject()) {
                QJsonObject jsonObj = jsonDoc.object();
                analyseJsonPacket(jsonObj);
            }
        } else {
            qDebug() << "Json parse error";
            return;
        }
    } else if (hdr->type == QD_MESSAGE_TYPE_LOG) {
        mWin->getStateInstance()->displayLog(QString(msg_body));
    } else {
        qDebug() << "Recv data type " << hdr->type << " not support";
    }
}

void Controller::analyseJsonPacket(QJsonObject &data) {
    QJsonObject extra;
    QString from, to, msgType;
    qint64 tick = getElapsedTimeMs();
    int cmd, seq, status;

    qDebug() << "Received json data: " << data;

    if (!data.contains("from") || !data.contains("to") ||
        !data.contains("cmd") || !data.contains("seq") ||
        !data.contains("result") || !data.contains("type")) {
        qDebug() << "No from or to or cmd or seq or type or result found";
        return;
    }

    to = data.value("to").toString();
    if (to != CENTER_NAME) {
        return;
    }

    from = data.value("from").toString();
    if (mEndSet.find(from.toStdString()) == mEndSet.end()) {
        qDebug() << "Not support data from " << from;
        return;
    }

    mEndSet[from.toStdString()]->updateEndConnectState(true, tick); //没断连

    msgType = data.value("type").toString();
    cmd = data.value("cmd").toInt();
    seq = data.value("seq").toInt();
    status = data.value("result").toInt();

    if (msgType == "request") {
        // 处理来自外部的请求
    } else {
        mEndSet[from.toStdString()]->updateEndExeState(cmd, seq, status);
        if (data.contains("extra")) {
            extra = data.value("extra").toObject();
            mEndSet[from.toStdString()]->parseExtraInfo(cmd, extra);
        }
    }
}
