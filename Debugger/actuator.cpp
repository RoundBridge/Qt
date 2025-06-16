#include <string.h>
#include "crc.h"
#include "controller.h"
#include "actuator.h"

Actuator::Actuator(Controller* c, const char* remoteIp, quint16 remotePort, Link* link, int id):End(c, id) {
    mCtrl = c;
    mCmd = 0, mSeq = 1;
    mContinueStrip = false;
    mRemotePort = remotePort;
    mRemoteIp.setAddress(remoteIp);
    mLink = link;
    memset(&mRemoteState, 0, sizeof(mRemoteState));
}

bool Actuator::setParam(uint32_t key, void* data, uint32_t dataLen) {
    bool ret = false;

    if (!data || dataLen == 0)
        return false;

    switch (key) {
    case CONTINUE_STRIP:
        if (dataLen >= sizeof(bool)) {
            mContinueStrip = *((bool*)data);
            ret = true;
        }
        break;
    default:
        break;
    }
    return ret;
}

bool Actuator::getParam(uint32_t key, void* data, uint32_t dataLen) {
    (void)key; (void)data; (void)dataLen;
    return false;
}

void Actuator::parseExtraInfo(uint32_t endCmd, QJsonObject &e) {
    if (endCmd == CMD_MAIN_ACTUATOR_QUERY) {
        parseQueryInfo(e);
    }
}

void Actuator::updateEndExeState(uint32_t endCmd, uint32_t seq, uint32_t state) {
    qDebug() << "Actuator current cmd " << mCmd << " seq " << mSeq << ", response cmd " << endCmd << " seq " << seq << " state " << state;
    if (mCmd == endCmd && mSeq+1 == seq) {
        mExeState = state & 0xF;
        if (mExeState == EXE_FAIL || mExeState == EXE_ABNORMAL) {
            // 获取错误码，后续再说
        }
    }
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
    case CTRL_PREPARE_STRIP:
        return CMD_MAIN_ACTUATOR_PREPARE_STRIP;
    case CTRL_STRIP:
        return CMD_MAIN_ACTUATOR_DO_STRIP;
    default:
        break;
    }
    return 0;
}

bool Actuator::processCmd(uint32_t ctrlCmd) {
    uint32_t mCmd = getMappedCmd(ctrlCmd);

    // 这里对当前的执行状态不做过滤了，即不管当前处于什么状态
    // 都往下发命令，由机头去过滤，因此，记录的就是最新的命令

    mExeState = EXE_WAIT;
    switch (mCmd) {
    case CMD_MAIN_ACTUATOR_STOP:
        return stop();
    case CMD_MAIN_ACTUATOR_RECOVER:
        return recover();
    case CMD_MAIN_ACTUATOR_PAUSE:
        return pause();
    case CMD_MAIN_ACTUATOR_RESUME:
        return resume();
    case CMD_MAIN_ACTUATOR_PREPARE_STRIP:
        return prepareStrip();
    case CMD_MAIN_ACTUATOR_DO_STRIP:
        return strip();
    default:
        mExeState = EXE_FAIL;
        qDebug() << "Controller cmd " << ctrlCmd << " not support";
        break;
    }
    return false;
}

void Actuator::parseStripperQueryInfo(QJsonObject& e) {
    qulonglong motorCurrent = e.value("motorCurrent").toVariant().toULongLong();
    mRemoteState.motorStripCurrent = static_cast<int16_t>(motorCurrent & 0xffff);
    mRemoteState.motorClampCurrent = static_cast<int16_t>((motorCurrent >> 32) & 0xffff);
    mRemoteState.motorKnifeCurrent = static_cast<int16_t>((motorCurrent >> 16) & 0xffff);

    uint32_t bat = static_cast<uint32_t>(e.value("bat").toInt());
    mRemoteState.voltageStrip = static_cast<int32_t>((bat >> 8) & 0xffffff);
}

void Actuator::parseQueryInfo(QJsonObject& e) {
    if (e.contains("stripper")) {
        QJsonObject stripperExtra = e.value("stripper").toObject();
        parseStripperQueryInfo(stripperExtra);
    }
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

bool Actuator::prepareStrip() {
    bool ret;
    QJsonObject e;
    QByteArray b;
    mSeq++;
    ret = makeCmdAndSend(CMD_MAIN_ACTUATOR_PREPARE_STRIP, QD_MESSAGE_TYPE_JSON, e, b);
    qDebug() << "prepare strip executed";
    return ret;
}

bool Actuator::strip() {
    bool ret;
    QJsonObject e;
    QByteArray b;
    mSeq++;
    ret = makeCmdAndSend(CMD_MAIN_ACTUATOR_DO_STRIP, QD_MESSAGE_TYPE_JSON, e, b);
    qDebug() << "strip executed";
    return ret;
}

bool Actuator::query() {
    bool ret;
    QJsonObject e;
    QByteArray b;

    if (mContinueStrip) {
        e.insert("skip", mContinueStrip);
    }
    ret = makeCmdAndSend(CMD_MAIN_ACTUATOR_QUERY, QD_MESSAGE_TYPE_JSON, e, b);
    return ret;
}
