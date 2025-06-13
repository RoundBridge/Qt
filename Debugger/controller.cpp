#include <QDebug>
#include "crc.h"
#include "controller.h"

Controller::Controller(QObject *parent)
    : QObject{parent}
{
    mCtrlCmd = 0;
    mIsStop = mIsPause = false;
    memset(mEndSet, 0, sizeof(mEndSet));
    memset(mLinkSet, 0, sizeof(mLinkSet));

    for (int i = Link_UDP; i < Link_num; ++i) {
        mLinkSet[i] = Link::createLink(i, 0);
    }

    mEndSet[End_actuator] = End::createEnd(this, mLinkSet[Link_UDP], End_actuator);

    mElapsedTimer.start();

    mPoller = new std::thread(&Controller::poll, this);
    mPoller->detach();

    connect(&mQueryTimer, &QTimer::timeout, this, &Controller::query);
    mQueryTimer.setSingleShot(false);
    mQueryTimer.start(15000);
}

bool Controller::setParam(int32_t end, uint32_t key, void* data, uint32_t dataLen) {
    if (mEndSet[end] && data && dataLen > 0) {
        return mEndSet[end]->setParam(key, data, dataLen);
    }
    return false;
}

bool Controller::getParam(int32_t end, uint32_t key, void* data, uint32_t dataLen) {
    if (mEndSet[end] && data && dataLen > 0) {
        return mEndSet[end]->getParam(key, data, dataLen);
    }
    return false;
}

bool Controller::dealCmd(uint32_t cmd, int32_t end) {
    bool ret = true;

    mIsStop = cmd == CTRL_STOP ? true : false;
    mIsPause = cmd == CTRL_PAUSE ? true : false;
    mCtrlCmd = cmd;

    if (end >= End_actuator && end < End_num) {
        if (!mEndSet[end]) {
            qDebug() << "End " << end << " not exist";
            return false;
        }
        return mEndSet[end]->execute(cmd);
    }

    for (int i = 0; i < End_num; ++i) {
        if (!mEndSet[i]) {
            continue;
        }
        if (!mEndSet[i]->execute(cmd)) {
            ret = false;
        }
    }
    return ret;
}

bool Controller::query() {
    qint64 tick = getElapsedTimeMs();

    for (int i = 0; i < End_num; ++i) {
        if (!mEndSet[i]) {
            continue;
        }
        mEndSet[i]->updateEndConnectState(false, tick); //检测是否断连
        mEndSet[i]->query();
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

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(msg_body, &error);

    if (hdr->type == QD_MESSAGE_TYPE_JSON) {
        if (error.error == QJsonParseError::NoError) {
            if (jsonDoc.isObject()) {
                QJsonObject jsonObj = jsonDoc.object();
                analyseJsonPacket(jsonObj);
            }
        } else {
            qDebug() << "Json parse error";
            return;
        }
    } else {
        qDebug() << "Recv data type " << hdr->type << " not support";
    }
}

void Controller::analyseJsonPacket(QJsonObject &data) {
    QJsonObject extra;
    QString from, to, msgType;
    qint64 tick = getElapsedTimeMs();
    int end = End_num, cmd, seq, status;

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
    if (from == MAIN_ACTUATOR_NAME) {
        end = End_actuator;
    } else {
        qDebug() << "Not support data from " << from;
        return;
    }

    mEndSet[end]->updateEndConnectState(true, tick); //没断连

    msgType = data.value("type").toString();
    cmd = data.value("cmd").toInt();
    seq = data.value("seq").toInt();
    status = data.value("result").toInt();

    mEndSet[end]->updateEndExeState(cmd, seq, status);

    if (msgType == "request") {
        // 处理来自外部的请求
    } else {
        if (data.contains("extra")) {
            extra = data.value("extra").toObject();
            mEndSet[end]->parseExtraInfo(cmd, extra);
        }
    }
}
