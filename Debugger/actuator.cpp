#include <string.h>
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

bool Actuator::stop() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(CMD_MAIN_ACTUATOR_STOP, ++mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "stop actuator executed";
        ret = true;
    }
    return ret;
}

bool Actuator::recover() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(CMD_MAIN_ACTUATOR_RECOVER, ++mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "recover actuator executed";
        ret = true;
    }
    return ret;
}

bool Actuator::pause() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(CMD_MAIN_ACTUATOR_PAUSE, ++mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "pause actuator executed";
        ret = true;
    }
    return ret;
}

bool Actuator::resume() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(CMD_MAIN_ACTUATOR_RESUME, ++mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "resume actuator executed";
        ret = true;
    }
    return ret;
}

bool Actuator::prepareStrip() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(CMD_MAIN_ACTUATOR_PREPARE_STRIP, ++mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "prepare strip executed";
        ret = true;
    }
    return ret;
}

bool Actuator::strip() {
    bool ret = false;
    uint32_t len = 0;
    QJsonObject e;
    QByteArray out;

    if (mContinueStrip) {
        e.insert("skip", mContinueStrip);
    }

    len = makeCmd(CMD_MAIN_ACTUATOR_DO_STRIP, ++mSeq, QD_MESSAGE_TYPE_JSON, out, &e);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "strip executed";
        ret = true;
    }
    return ret;
}

bool Actuator::query() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(CMD_MAIN_ACTUATOR_QUERY, mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        ret = true;
    }
    return ret;
}
