#ifndef JOINT_H
#define JOINT_H

#include "common.h"
#include "end.h"


typedef enum {
    //集成动作命令
    CMD_JOINT_RESET = 1,            //不支持

    //机头水平旋转（面对绝缘杆，电路板侧为左侧），带参数，"pattern"，类型int，1-逆时针连续转动，2-顺时针连续转动，3-转特定角度，
    //"angle"，类型double，正数顺时针转，负数逆时针转（单位，度），在pattern为3时使用。
    //"speed"，类型float，单位（r/s，每秒圈数），表示旋转的速度(可选)
    //"current"，类型float，单位（mA），(可选)
    CMD_JOINT_ROTATE,
    CMD_JOINT_ROTATE_STOP,
    //绝缘杆上下摆动（面对绝缘杆，电路板侧为左侧），带参数，"pattern"，类型int，1-上摆，2-下摆，3-摆动特定角度，
    //"angle"，类型double，正数上摆，负数下摆（单位，度），在pattern为3时使用。
    //"speed"，类型float，单位（每秒度数），表示摆动的速度(可选)
    //"current"，类型float，单位（mA），(可选)
    CMD_JOINT_PITCH,
    CMD_JOINT_PITCH_STOP,

    // extra 规则：
    // "module": 代表的模块，"joint"...
    // "type": 代表操作的类型，"motor"
    // "tid": （int32类型）当"type"是"motor"类型时，id号表示电机id号的组合（位或方式），各个电机的ID号如下：
    //         8：旋转电机，9：摆动电机
    //         比如要开启或者关闭这两个电机的电源，则 tid = (1 << 8) || (1 << 9)
    CMD_JOINT_POWER_ON,
    CMD_JOINT_POWER_OFF,
    // "module": 代表的模块，"joint"
    CMD_JOINT_REBOOT,
    // extra 规则：
    // "module": 代表的模块，"joint"
    // "tid": id号(1-旋转标定，2-摆动标定)
    CMD_JOINT_CALIBRATION,              //不支持

    // 如果使用过程中角度异常，会触发微动硬件断电保护，此时利用下面两个命令给电机恢复上电
    CMD_JOINT_ROTATE_HARD_RECOVER,      //旋转电机恢复上电(暂不支持)
    CMD_JOINT_PITCH_HARD_RECOVER,       //俯仰电机恢复上电

    CMD_JOINT_FIRMWARE_UPDATE,          //不支持

    /**
     * 参数设置通用接口
     * extra 规则：
     *
     * "key": int32类型
     * "value": int32类型，>0表示要设置的值，==0表示查询当前的值以及设备默认值，<0无效
     *
     *          key = 1：设置旋转电机是否强制运动，value = 1 表示强制运动，value = 2 表示非强制运动（危险动作，不要轻易使用，主要用于急救，比如光耦坏了电机断电了）
     *          key = 2：设置摆动电机是否强制运动，value = 1 表示强制运动，value = 2 表示非强制运动（危险动作，不要轻易使用，主要用于急救，比如光耦坏了电机断电了）
     *          key = 3：设置摆动电机最大摆动角度，value 表示摆动角度的10倍，比如设置76.5度，则 value = 765，回复的 value 和 default 也放大了10倍
     * 回复："key": int32类型，原封不动地将主控传过来的值返回回去，"value"，int32类型，表示设置前的值，"default"，int32类型，表示设备默认值
     */
    CMD_JOINT_SET_PARAMS = 0x99,        //不支持


    CMD_JOINT_PACKING = 0x9F,           //不支持
    CMD_JOINT_QUERY = 0xA0,
    CMD_JOINT_PAUSE,
    CMD_JOINT_STOP,
    CMD_JOINT_RECOVER,
    CMD_JOINT_RESUME,
    CMD_JOINT_QUIT,                     //不支持

    // extra 规则：
    CMD_TEST = 0xff                     //不支持
} JointCmd;

class joint : public End
{
public:
    joint(Controller* c, const char* remoteIp, quint16 remotePort, Link* link, int id = End_joint);

    virtual uint32_t getMappedCmd(uint32_t ctrlCmd);
    virtual bool processCmd(uint32_t ctrlCmd);
    virtual bool setParam(uint32_t key, void* data, uint32_t dataLen);
    virtual bool getParam(uint32_t key, void* data, uint32_t dataLen);

private:
    bool stop();
    bool recover();
    bool pause();
    bool resume();
    bool reset();
    bool rotate();
    bool pitch();
    bool stop_rotate();
    bool stop_pitch();

private:
    uint32_t mCmd, mSeq;
    quint16 mRemotePort;
    QHostAddress mRemoteIp;
    Link* mLink;
};

#endif // JOINT_H
