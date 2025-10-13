#include "controller.h"
#include "joint.h"

joint::joint(Controller* c, const char* remoteIp, quint16 remotePort, Link* link, const std::string& id):End(c, remoteIp, remotePort, link, id) {

}

bool joint::setParam(uint32_t key, void* data, uint32_t dataLen) {
    bool ret = false;

    if (!data || dataLen == 0)
        return false;

    switch (key) {
    case JOINTOR_ROTATE_PARAMS:
        break;
    default:
        break;
    }
    return ret;
}

bool joint::getParam(uint32_t key, void* data, uint32_t dataLen) {
    (void)key; (void)data; (void)dataLen;
    return false;
}

uint32_t joint::getMappedCmd(uint32_t ctrlCmd) {
    switch (ctrlCmd) {
    case CTRL_STOP:
        return CMD_JOINT_STOP;
    case CTRL_RECOVER:
        return CMD_JOINT_RECOVER;
    case CTRL_PAUSE:
        return CMD_JOINT_PAUSE;
    case CTRL_RESUME:
        return CMD_JOINT_RESUME;
    case CTRL_RESET:
        return CMD_JOINT_RESET;
    case CTRL_JOINT_ROTATE:
        return CMD_JOINT_ROTATE;
    case CTRL_JOINT_ROTATE_STOP:
        return CMD_JOINT_ROTATE_STOP;
    case CTRL_JOINT_PITCH:
        return CMD_JOINT_PITCH;
    case CTRL_JOINT_PITCH_STOP:
        return CMD_JOINT_PITCH_STOP;
    default:
        break;
    }
    qDebug() << "joint not support ctrl cmd " << ctrlCmd;
    return MAX_UINT32;
}

bool joint::processCmd(uint32_t ctrlCmd) {
    uint32_t mCmd = getMappedCmd(ctrlCmd);

    // 这里对当前的执行状态不做过滤了，即不管当前处于什么状态
    // 都往下发命令，由机头去过滤，因此，记录的就是最新的命令

    mExeState = EXE_WAIT;
    switch (mCmd) {
    case CMD_JOINT_STOP:
        return stop();
    case CMD_JOINT_RECOVER:
        return recover();
    case CMD_JOINT_PAUSE:
        return pause();
    case CMD_JOINT_RESUME:
        return resume();
    case CMD_JOINT_RESET:
        return reset();
    case CMD_JOINT_ROTATE:
        return rotate();
    case CMD_JOINT_ROTATE_STOP:
        return stop_rotate();
    case CMD_JOINT_PITCH:
        return pitch();
    case CMD_JOINT_PITCH_STOP:
        return stop_pitch();
    default:
        mExeState = EXE_FAIL;
        qDebug() << "Controller cmd " << ctrlCmd << " not support";
        break;
    }
    return false;
}

bool joint::rotate() {
    return false;
}

bool joint::stop_rotate() {
    return false;
}

bool joint::pitch() {
    return false;
}

bool joint::stop_pitch() {
    return false;
}
