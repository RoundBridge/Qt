#include "attitude.h"
#include "controller.h"

attitude::attitude(Controller* c, const char* remoteIp, quint16 remotePort, Link* link, const std::string& id):End(c, remoteIp, remotePort, link, id) {

}

bool attitude::setParam(uint32_t key, void* data, uint32_t dataLen) {
    bool ret = true;

    if (!data || dataLen == 0)
        return false;

    switch (key) {
    case SET_ATTITUDE_ROTATE_PATTERN:
        mRotatePattern = *((int32_t*)data);
        qDebug() << "attitude set rotate pattern " << mRotatePattern;
        break;
    case SET_ATTITUDE_ROTATE_ANGLE:
        mRotateAngle = *((float*)data);
        qDebug() << "attitude set rotate angle:" << mRotateAngle;
        break;
    case SET_ATTITUDE_ROTATE_SPEED:
        mRotateSpeed = *((float*)data);
        qDebug() << "attitude set rotate speed:" << mRotateSpeed;
        break;
    case SET_ATTITUDE_ROTATE_CURRENT:
        mRotateCurrent = *((float*)data);
        qDebug() << "attitude set rotate current:" << mRotateCurrent;
        break;

    case SET_ATTITUDE_PITCH_PATTERN:
        mPitchPattern = *((int32_t*)data);
        qDebug() << "attitude set pitch pattern " << mPitchPattern;
        break;
    case SET_ATTITUDE_PITCH_CURRENT:
        mPitchCurrent = *((float*)data);
        qDebug() << "attitude set pitch current:" << mPitchCurrent;
        break;
    case SET_ATTITUDE_PITCH_SPEED:
        mPitchSpeed = *((float*)data);
        qDebug() << "attitude set pitch speed:" << mPitchSpeed;
        break;
    case SET_ATTITUDE_PITCH_ANGLE:
        mPitchAngle = *((float*)data);
        qDebug() << "attitude set pitch angle:" << mPitchAngle;
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}

bool attitude::getParam(uint32_t key, void* data, uint32_t dataLen) {
    (void)key; (void)data; (void)dataLen;
    return false;
}

uint32_t attitude::getMappedCmd(uint32_t ctrlCmd) {
    switch (ctrlCmd) {
    case CTRL_STOP:
        return CMD_ATTITUDE_STOP;
    case CTRL_RECOVER:
        return CMD_ATTITUDE_RECOVER;
    case CTRL_PAUSE:
        return CMD_ATTITUDE_PAUSE;
    case CTRL_RESUME:
        return CMD_ATTITUDE_RESUME;
    case CTRL_RESET:
        return CMD_ATTITUDE_RESET;
    case CTRL_ATTITUDE_ROTATE:
        return CMD_ATTITUDE_ROTATE;
    case CTRL_ATTITUDE_ROTATE_STOP:
        return CMD_ATTITUDE_ROTATE_STOP;
    case CTRL_ATTITUDE_PITCH:
        return CMD_ATTITUDE_PITCH;
    case CTRL_ATTITUDE_PITCH_STOP:
        return CMD_ATTITUDE_PITCH_STOP;
    case CTRL_ATTITUDE_YAW:
    case CTRL_ATTITUDE_YAW_STOP:
    default:
        break;
    }
    qDebug() << "attitude not support ctrl cmd " << ctrlCmd;
    return MAX_UINT32;
}

bool attitude::processCmd(uint32_t ctrlCmd) {
    uint32_t mCmd = getMappedCmd(ctrlCmd);

    // 这里对当前的执行状态不做过滤了，即不管当前处于什么状态
    // 都往下发命令，由机头去过滤，因此，记录的就是最新的命令

    mExeState = EXE_WAIT;
    switch (mCmd) {
    case CMD_ATTITUDE_STOP:
        return stop();
    case CMD_ATTITUDE_RECOVER:
        return recover();
    case CMD_ATTITUDE_PAUSE:
        return pause();
    case CMD_ATTITUDE_RESUME:
        return resume();
    case CMD_ATTITUDE_RESET:
        return reset();
    case CMD_ATTITUDE_ROTATE:
        return rotate();
    case CMD_ATTITUDE_ROTATE_STOP:
        return stop_rotate();
    case CMD_ATTITUDE_PITCH:
        return pitch();
    case CMD_ATTITUDE_PITCH_STOP:
        return stop_pitch();
    default:
        mExeState = EXE_FAIL;
        qDebug() << "Controller cmd " << ctrlCmd << " not support";
        break;
    }
    return false;
}

bool attitude::rotate() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;
    QJsonObject extra;

    extra.insert("pattern", mRotatePattern);
    extra.insert("angle", mRotateAngle);
    extra.insert("speed", mRotateSpeed/360.0); //旋转命令中，speed是每秒圈数
    extra.insert("current", mRotateCurrent);

    len = makeCmd(CMD_ATTITUDE_ROTATE, ++mSeq, QD_MESSAGE_TYPE_JSON, out, &extra);

    // if (len) {
    //     QByteArray msg_body = out.mid(sizeof(QDMessageHdr), out.size() - sizeof(QDMessageHdr));
    //     QJsonDocument jsonDoc = QJsonDocument::fromJson(msg_body);
    //     QString formattedJson = jsonDoc.toJson(QJsonDocument::Indented);
    //     qDebug().noquote() << formattedJson;
    // }

    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "attitude rotate executed";
        ret = true;
    }
    return ret;
}

bool attitude::stop_rotate() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(CMD_ATTITUDE_ROTATE_STOP, ++mSeq, QD_MESSAGE_TYPE_JSON, out);

    // if (len) {
    //     QByteArray msg_body = out.mid(sizeof(QDMessageHdr), out.size() - sizeof(QDMessageHdr));
    //     QJsonDocument jsonDoc = QJsonDocument::fromJson(msg_body);
    //     QString formattedJson = jsonDoc.toJson(QJsonDocument::Indented);
    //     qDebug().noquote() << formattedJson;
    // }

    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "attitude rotate stop executed";
        ret = true;
    }
    return ret;
}

bool attitude::pitch() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;
    QJsonObject extra;

    extra.insert("pattern", mPitchPattern);
    extra.insert("angle", mPitchAngle);
    extra.insert("speed", mPitchSpeed); //摆动命令中，speed是每秒度数
    extra.insert("current", mPitchCurrent);

    len = makeCmd(CMD_ATTITUDE_PITCH, ++mSeq, QD_MESSAGE_TYPE_JSON, out, &extra);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "attitude rotate executed";
        ret = true;
    }
    return ret;
}

bool attitude::stop_pitch() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(CMD_ATTITUDE_PITCH_STOP, ++mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "attitude pitch stop executed";
        ret = true;
    }
    return ret;
}
