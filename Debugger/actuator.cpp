#include "crc.h"
#include "controller.h"
#include "actuator.h"

Actuator::Actuator(Controller* c, const char* remoteIp, quint16 remotePort, Link* link, int id):End(c, id) {
    mCtrl = c;
    mCmd = 0, mSeq = 1;
    mRemotePort = remotePort;
    mRemoteIp.setAddress(remoteIp);
    mLink = link;
}

uint32_t Actuator::getMappedCmd(uint32_t ctrlCmd) {
    switch (ctrlCmd) {
    case CTRL_STOP:
        return CMD_MAIN_ACTUATOR_STOP;
    case CTRL_RECOVER:
        return CMD_MAIN_ACTUATOR_RECOVER;
    case CTRL_PAUSE:
        return CMD_MAIN_ACTUATOR_PAUSE;
    case CTRL_RESUME:
        return CMD_MAIN_ACTUATOR_RESUME;
    case CTRL_RESET:
        return CMD_MAIN_ACTUATOR_RESET;
    default:
        break;
    }
    return 0;
}

bool Actuator::processCmd(uint32_t ctrlCmd) {
    uint32_t cmd = getMappedCmd(ctrlCmd);

    switch (cmd) {
    case CMD_MAIN_ACTUATOR_STOP:
        return stop();
    case CMD_MAIN_ACTUATOR_RECOVER:
        return recover();
    case CMD_MAIN_ACTUATOR_PAUSE:
        return pause();
    case CMD_MAIN_ACTUATOR_RESUME:
        return resume();
    default:
        qDebug() << "Controller cmd " << ctrlCmd << " not support";
        break;
    }
    return false;
}

bool Actuator::reConnect() {
    bool ret = false;

    mConnectState = Link_Connecting;

    if (mLink)
        ret = mLink->reset();

    if (ret) {
        mConnectState = Link_Connect;
    } else {
        mConnectState = Link_Disconnect;
    }
    return ret;
}

bool Actuator::makeCmdAndSend(uint32_t c, int32_t msgType, QJsonObject& extra, QByteArray& byte) {
    uint32_t bodyL = 0, hdrL = 0;
    QDMessageHdr hdr;
    QJsonObject root;
    QJsonDocument doc;
    QByteArray array;
    QByteArray crc_data;
    QByteArray send_data_head1;
    QByteArray send_data_head2;
    QByteArray send_data;

    if (!mLink) {
        qDebug() << "Actuator link not init!";
        return false;
    }

    hdrL = sizeof(QDMessageHdr);

    if (msgType == QD_MESSAGE_TYPE_JSON) {
        root.insert("from", CENTER_NAME);
        root.insert("to", MAIN_ACTUATOR_NAME);
        root.insert("type", "request");
        root.insert("cmd", QJsonValue::fromVariant(QVariant(c)));
        root.insert("result", 1);
        root["seq"] = QJsonValue::fromVariant(QVariant(mSeq));
        root.insert("extra", extra);
        doc.setObject(root);
        array = doc.toJson(QJsonDocument::Compact);
    } else {
        array = byte;
    }
    bodyL = array.size();

    hdr.magic = 0x4E5A4451;
    hdr.magic = qToBigEndian(hdr.magic);
    hdr.len = bodyL + hdrL;
    hdr.len = qToBigEndian(hdr.len);
    hdr.type = msgType;
    hdr.type = qToBigEndian(hdr.type);

    QDataStream out1(&send_data_head1, QIODevice::WriteOnly);
    QDataStream out2(&send_data_head2, QIODevice::WriteOnly);

    out1 << hdr.len << hdr.type;
    crc_data = send_data_head1 + array;
    hdr.crc32 = getCRC32((unsigned char *)(crc_data.data()), bodyL + 8);
    hdr.crc32 = qToBigEndian(hdr.crc32);
    out2 << hdr.magic << hdr.crc32;
    send_data = send_data_head2 + crc_data;

    if (c != CMD_MAIN_ACTUATOR_QUERY && msgType == QD_MESSAGE_TYPE_JSON)
        qDebug() << "msg len " << bodyL + hdrL << ", type " << msgType << ", content:" << array;

    return mLink->send((uint8_t*)send_data.data(), (uint32_t)send_data.size(), mRemoteIp, mRemotePort);
}

bool Actuator::stop() {
    bool ret;
    QJsonObject e;
    QByteArray b;
    mSeq++;
    ret = makeCmdAndSend(CMD_MAIN_ACTUATOR_STOP, QD_MESSAGE_TYPE_JSON, e, b);
    qDebug() << "stop actuator executed";
    return ret;
}

bool Actuator::recover() {
    bool ret;
    QJsonObject e;
    QByteArray b;
    mSeq++;
    ret = makeCmdAndSend(CMD_MAIN_ACTUATOR_RECOVER, QD_MESSAGE_TYPE_JSON, e, b);
    qDebug() << "recover actuator executed";
    return ret;
}

bool Actuator::pause() {
    bool ret;
    QJsonObject e;
    QByteArray b;
    mSeq++;
    ret = makeCmdAndSend(CMD_MAIN_ACTUATOR_PAUSE, QD_MESSAGE_TYPE_JSON, e, b);
    qDebug() << "pause actuator executed";
    return ret;
}

bool Actuator::resume() {
    bool ret;
    QJsonObject e;
    QByteArray b;
    mSeq++;
    ret = makeCmdAndSend(CMD_MAIN_ACTUATOR_RESUME, QD_MESSAGE_TYPE_JSON, e, b);
    qDebug() << "resume actuator executed";
    return ret;
}

bool Actuator::query() {
    bool ret;
    QJsonObject e;
    QByteArray b;
    ret = makeCmdAndSend(CMD_MAIN_ACTUATOR_QUERY, QD_MESSAGE_TYPE_JSON, e, b);
    return ret;
}
