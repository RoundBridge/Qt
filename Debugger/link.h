#ifndef LINK_H
#define LINK_H

#include <stdint.h>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QtNetwork>


/*
 * 暂时只支持UDP，串口、TCP什么的后续视情况再支持
 */
typedef enum {
    Link_UDP = 0,
    Link_TCP_Server,
    Link_TCP_Client,
    Link_num = Link_TCP_Server
} LinkType;

typedef enum {
    Link_Unknown = 0,
    Link_Connect,
    Link_Connecting,
    Link_Disconnect
} LinkState;

/*
 * 通信连接的基类，实际的网络通信/串口通信等等都继承自此类
 */
class Link
{
public:
    Link();
    virtual ~Link();
    static Link* createLink(int type, quint16 localPort = 0);
    static void destroyLink(Link* l);
    virtual bool reset();
    virtual bool send(uint8_t* data, uint32_t len);
    virtual bool send(uint8_t* data, uint32_t len, QHostAddress ip, quint16 port);
};

class Udp : public Link
{
public:
    Udp(quint16 localPort = 0);
    virtual ~Udp();
    virtual bool send(uint8_t* data, uint32_t len, QHostAddress ip, quint16 port);

private:
    QUdpSocket *mSocket;
    quint16 mLocalPort;
};

#endif // LINK_H
