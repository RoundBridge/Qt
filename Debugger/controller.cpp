#include <QDebug>
#include "controller.h"

Controller::Controller(QObject *parent)
    : QObject{parent}
{
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

bool Controller::dealCmd(uint32_t cmd, int32_t end) {
    bool ret = true;

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
    for (int i = 0; i < End_num; ++i) {
        if (!mEndSet[i]) {
            continue;
        }
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
            }
        }
    }
}
