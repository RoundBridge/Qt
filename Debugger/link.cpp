#include <QDebug>
#include "link.h"

Link::Link() {

}

Link::~Link() {

}

Link* Link::createLink(int type, quint16 localPort) {
    Link* l = nullptr;
    if (type == Link_UDP) {
        l = new Udp(localPort);
    }
    return l;
}

void Link::destroyLink(Link* l) {
    delete l;
}

bool Link::reset() {
    return false;
}

bool Link::send(uint8_t* data, uint32_t len) {
    (void)data; (void)len;
    return false;
}

bool Link::send(uint8_t* data, uint32_t len, QHostAddress ip, quint16 port) {
    (void)data; (void)len; (void)ip; (void)port;
    return false;
}

Udp::Udp(quint16 localPort) {
    mLocalPort = localPort == 0 ? 6666 : localPort;
    mSocket = new QUdpSocket(nullptr);
}

Udp::~Udp() {
    delete mSocket;
}

bool Udp::send(uint8_t* data, uint32_t len, QHostAddress ip, quint16 port) {
    qint64 ret = mSocket->writeDatagram((const char*)data, len, ip, port);
    if (ret != len) {
        qDebug() << "Udp send to " << ip << " port " << port << " failed, total " << len << " bytes, send " << ret << " bytes";
        return false;
    } else {
        return true;
    }
}
