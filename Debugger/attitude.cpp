#include "attitude.h"
#include "controller.h"

attitude::attitude(Controller* c, const char* remoteIp, quint16 remotePort, Link* link, const std::string& id):End(c, remoteIp, remotePort, link, id) {

}

bool attitude::setParam(uint32_t key, void* data, uint32_t dataLen) {
    bool ret = false;

    if (!data || dataLen == 0)
        return false;

    switch (key) {
    case ATTITUDE_ROTATE_PARAMS:
        break;
    default:
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
    return false;
}

bool attitude::pitch() {
    return false;
}

bool attitude::stop_rotate() {
    return false;
}

bool attitude::stop_pitch() {
    return false;
}
