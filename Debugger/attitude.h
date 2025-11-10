#ifndef ATTITUDE_H
#define ATTITUDE_H

#include "common.h"
#include "end.h"


typedef enum {
    //集成动作命令
    CMD_ATTITUDE_RESET = 1,            //不支持

    //机头水平旋转，带参数，"pattern"，类型int，1-逆时针连续转动，2-顺时针连续转动，3-转特定角度，
    //"angle"，类型double，正数顺时针转，负数逆时针转（单位，度），在pattern为3时使用。
    //"speed"，类型float，单位（r/s，每秒圈数），表示旋转的速度(可选)
    //"current"，类型float，单位（mA），(可选)
    CMD_ATTITUDE_ROTATE,
    CMD_ATTITUDE_ROTATE_STOP,
    //电池侧为左侧
    //机头左右上下摆动，带参数，"pattern"，类型int，1-向后摆（机头远离观察者，角度减小），2-向前摆（机头靠近观察者，角度增大），3-摆动特定角度，
    //"angle"，类型double，正数向前摆（机头靠近观察者，角度增大），负数向后摆（机头远离观察者，角度减小）（单位，度），在pattern为3时使用。
    //"speed"，类型float，单位（每秒度数），表示摆动的速度(可选)
    //"current"，类型float，单位（mA），(可选)
    CMD_ATTITUDE_PITCH,
    CMD_ATTITUDE_PITCH_STOP,

    //电池侧为左侧
    //机头左右上下偏转，带参数，"pattern"，类型int，1-电池侧上升，角度增大，2-电池侧下降，角度减小，3-摆动特定角度，
    //"angle"，类型double，正数电池侧上升，角度增大，电池侧下降，角度减小（单位，度），在pattern为3时使用。
    //"speed"，类型float，单位（每秒度数），表示摆动的速度(可选)
    //"current"，类型float，单位（mA），(可选)
    CMD_ATTITUDE_YAW,
    CMD_ATTITUDE_YAW_STOP,

    // extra 规则：
    // "module": 代表的模块，"attitude"...
    // "type": 代表操作的类型，"motor"
    // "tid": （int32类型）当"type"是"motor"类型时，id号表示电机id号的组合（位或方式），各个电机的ID号如下：
    //         8：旋转电机，9：摆动电机，10：偏转电机
    //         比如要开启或者关闭这两个电机的电源，则 tid = (1 << 8) || (1 << 9)
    CMD_ATTITUDE_POWER_ON,
    CMD_ATTITUDE_POWER_OFF,
    // "module": 代表的模块，"attitude"
    CMD_ATTITUDE_REBOOT,
    // extra 规则：
    // "module": 代表的模块，"attitude"
    // "tid": id号(1-旋转标定，2-摆动标定)
    CMD_ATTITUDE_CALIBRATION,              //不支持

    // 如果使用过程中角度异常，会触发微动硬件断电保护，此时利用下面两个命令给电机恢复上电
    CMD_ATTITUDE_ROTATE_HARD_RECOVER,      //旋转电机恢复上电(暂不支持)
    CMD_ATTITUDE_PITCH_HARD_RECOVER,       //俯仰电机恢复上电

    CMD_ATTITUDE_FIRMWARE_UPDATE,          //不支持

    /**
     * 参数设置通用接口
     * extra 规则：
     *
     * "key": int32类型
     * "value": int32类型，>0表示要设置的值，==0表示查询当前的值以及设备默认值，<0无效
     *          key = 1：设置旋转电机是否强制运动，value = 1 表示强制运动，value = 2 表示非强制运动（危险动作，不要轻易使用，主要用于急救，比如光耦坏了电机断电了）
     *          key = 2：设置摆动电机是否强制运动，value = 1 表示强制运动，value = 2 表示非强制运动（危险动作，不要轻易使用，主要用于急救，比如光耦坏了电机断电了）
     *          key = 3：设置摆动电机最大摆动角度，value 表示摆动角度的10倍，比如设置76.5度，则 value = 765，回复的 value 和 default 也放大了10倍
     *          key = 4：设置偏转电机是否强制运动，value = 1 表示强制运动，value = 2 表示非强制运动（危险动作，不要轻易使用，主要用于急救，比如光耦坏了电机断电了）
     *          key = 5：设置偏转电机最大摆动角度，value 表示偏转角度的10倍，比如设置76.5度，则 value = 765，回复的 value 和 default 也放大了10倍
     * 回复："key": int32类型，原封不动地将主控传过来的值返回回去，"value"，int32类型，表示设置前的值，"default"，int32类型，表示设备默认值
     */
    CMD_ATTITUDE_SET_PARAMS = 0x99,        //不支持


    CMD_ATTITUDE_PACKING = 0x9F,           //不支持
    CMD_ATTITUDE_QUERY = 0xA0,
    CMD_ATTITUDE_PAUSE,
    CMD_ATTITUDE_STOP,
    CMD_ATTITUDE_RECOVER,
    CMD_ATTITUDE_RESUME,
    CMD_ATTITUDE_QUIT,                     //不支持

    // extra 规则：
    CMD_ATTITUDE_TEST = 0xff                        //不支持
} AttitudeCmd;

class attitude : public End
{
public:
    attitude(Controller* c, const char* remoteIp, quint16 remotePort, Link* link, const std::string& id = ATTITUDE_NAME);

    virtual uint32_t getMappedCmd(uint32_t ctrlCmd);
    virtual bool processCmd(uint32_t ctrlCmd);
    virtual bool setParam(uint32_t key, void* data, uint32_t dataLen);
    virtual bool getParam(uint32_t key, void* data, uint32_t dataLen);
    virtual bool doWorkProc(bool stop);

private:
    bool rotate();
    bool pitch();
    bool stop_rotate();
    bool stop_pitch();
    bool yaw();
    bool stop_yaw();

private:
    int32_t mRotatePattern = 3, mYawPattern = 3, mPitchPattern = 3;
    float mRotateCurrent = -1, mRotateSpeed = -1, mRotateAngle = 0;
    float mPitchCurrent = -1, mPitchSpeed = -1, mPitchAngle = 0;
    float mYawCurrent = -1, mYawSpeed = -1, mYawAngle = 0;
};

#endif // ATTITUDE_H
