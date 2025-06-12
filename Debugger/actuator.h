#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <QJsonObject>
#include <QJsonDocument>
#include "common.h"
#include "end.h"

typedef enum {
    //集成动作命令
    CMD_MAIN_ACTUATOR_RESET = 1,            //机头复位
    CMD_MAIN_ACTUATOR_START_PLACE_CLAMP,    //开始放线夹
    CMD_MAIN_ACTUATOR_STRIPPER_RESET,       //剥线器复位
    CMD_MAIN_ACTUATOR_PREPARE_UP,           //准备上线
    CMD_MAIN_ACTUATOR_INSERT_BYPASS,        //松开线夹，插入引流线
    CMD_MAIN_ACTUATOR_GRIP_CABEL,           //抓取母线
    CMD_MAIN_ACTUATOR_RELEASE_CABEL,        //松开母线
    CMD_MAIN_ACTUATOR_PREPARE_STRIP,        //准备剥线
    CMD_MAIN_ACTUATOR_DO_STRIP,             //剥线
    CMD_MAIN_ACTUATOR_SCREW,                //拧螺钉
    CMD_MAIN_ACTUATOR_PREPARE_DOWN,         //准备下线
    CMD_MAIN_ACTUATOR_WIRE_BREAKAGE_PROCESS,//断线处理

    //单步动作命令
    CMD_MAIN_ACTUATOR_DO_SCREW_SINGLE = 0x64,   //带参数，正表示正转圈数，负表示负转圈数
    CMD_MAIN_ACTUATOR_RELEASE_BYPASS,           //松开引流线
    CMD_MAIN_ACTUATOR_STOP_SCREW,               //停止拧螺钉
    CMD_MAIN_ACTUATOR_STRIPPER_MOVE,
    CMD_MAIN_ACTUATOR_CONNECTOR_MOVE,
    CMD_MAIN_ACTUATOR_KNIFE_MOVE,               // 1、退刀：参数"dist"发退刀的距离，单位毫米，负数，double
    // 2、进刀：参数"dist"发进刀的距离，单位毫米，正数，double
    CMD_MAIN_ACTUATOR_STOP_STRIPPER_MOVE,
    CMD_MAIN_ACTUATOR_STOP_CONNECTOR_MOVE,
    CMD_MAIN_ACTUATOR_STRIPPER_ROTATE,          //带参数，正表示正转圈数，负表示负转圈数,还有一个是全速和低速参数，"count" 类型double（单位圈），"fullSpeed" 类型bool
    CMD_MAIN_ACTUATOR_STOP_STRIPPER_ROTATE,
    CMD_MAIN_ACTUATOR_CLOSE_GRIPPER,
    CMD_MAIN_ACTUATOR_RELEASE_GRIPPER,

    //机头水平旋转，带参数，"pattern"，类型int，1-逆时针连续转动，2-顺时针连续转动，3-转特定角度，
    //"angle"，类型double，正数顺时针转，负数逆时针转（单位，度），在pattern为3时使用。
    //"speed"，类型int，表示旋转的速度
    CMD_MAIN_ACTUATOR_ROTATE,
    CMD_MAIN_ACTUATOR_ROTATE_STOP,
    //机头左右上下摆动，带参数，"pattern"，类型int，1-左侧连续上翘，2-右侧连续上翘，3-俯仰特定角度，
    //"angle"，类型double，正数左侧上翘，负数右侧上翘（单位，度），在pattern为3时使用。
    //"speed"，类型int，表示俯仰的速度
    CMD_MAIN_ACTUATOR_PITCH,
    CMD_MAIN_ACTUATOR_PITCH_STOP,

    // extra 规则：
    // "module": 代表的模块，"stripper"/"actuator"...
    // "type": 代表操作的类型，"motor"/"camera"...
    // "tid": id号
    CMD_MAIN_ACTUATOR_POWER_ON,
    CMD_MAIN_ACTUATOR_POWER_OFF,
    // "module": 代表的模块，"stripper"/"actuator"...
    CMD_MAIN_ACTUATOR_REBOOT,
    // extra 规则：
    // "module": 代表的模块，"stripper"/"actuator"...
    // "tid": id号(标定机头时，1-旋转标定，2-俯仰标定)
    CMD_MAIN_ACTUATOR_CALIBRATION,

    CMD_MAIN_STRIPPER_FIRMWARE_UPDATE,          //剥线器固件升级
    CMD_MAIN_ACTUATOR_FIRMWARE_UPDATE,          //机头固件升级

    /**
     * 参数设置通用接口
     * extra 规则：
     *
     * "key": int32类型，1：设置回路检测阈值
     * "value": int32类型，>0表示要设置的阈值，==0表示查询当前的阈值以及设备默认阈值，<0无效
     * 回复："key": int32类型，原封不动地将主控传过来的值返回回去，"value"，int32类型，表示设置前的阈值，"default"，int32类型，表示设备默认值
     */
    CMD_MAIN_ACTUATOR_SET_PARAMS = 0x99,

    CMD_MAIN_ACTUATOR_QUERY_NOISE = 0x9A,       //查询矩阵红外的底噪
    // 拧螺钉线性推杆运动，"dir": 1-向里回收，2-向外推出
    CMD_MAIN_ACTUATOR_MOVE_ROD = 0x9B,
    // 设置第一次暂停时的进刀距离，单位mm，类型double，字段"dist"
    CMD_MAIN_ACTUATOR_SET_KNIFE_FORWARD_DIST_BEFORE_FIRST_CHECK = 0x9C,

    CMD_MAIN_ACTUATOR_OPEN_WIRE_SKIN_BOX = 0x9D,//打开收线盒

    // 上线准备+机头旋转，参数：extra: "rotate" "-90"  ;  "rotate_speed"  "2000"; "pitch"  "0","pitch_speed" "500", int型
    CMD_MAIN_ACTUATOR_PREPARE_FOR_ALL_AUTO_PROCESS = 0x9E,

    CMD_MAIN_ACTUATOR_PACKING = 0x9F,           //机头装箱命令
    CMD_MAIN_ACTUATOR_QUERY = 0xA0,
    CMD_MAIN_ACTUATOR_PAUSE,
    CMD_MAIN_ACTUATOR_STOP,
    CMD_MAIN_ACTUATOR_RECOVER,
    CMD_MAIN_ACTUATOR_RESUME,
    CMD_MAIN_ACTUATOR_QUIT,                     //放弃当前命令执行

    CMD_MAIN_ACTUATOR_LOOP_AD = 0xB0,           //给主控发送回路检测AD数据
    CMD_MAIN_ACTUATOR_INFRARED_AD = 0xB1,       //给主控发送红外检测AD数据

    // extra 规则：
    // "test_cmd": int8，1-开启皮芯检测
    CMD_TEST = 0xff                             //测试命令
} ActuatorCmd;

class Actuator : public End
{
public:
    Actuator(Controller* c, const char* remoteIp, quint16 remotePort, Link* link, int id = End_actuator);

    virtual uint32_t getMappedCmd(uint32_t ctrlCmd);
    virtual bool processCmd(uint32_t ctrlCmd);
    virtual bool reConnect();
    virtual bool query();

private:
    bool stop();
    bool recover();
    bool pause();
    bool resume();
    bool makeCmdAndSend(uint32_t c, int32_t msgType, QJsonObject& extra, QByteArray& byte);

private:
    uint32_t mCmd, mSeq;
    quint16 mRemotePort;
    QHostAddress mRemoteIp;
    Link* mLink;
};

#endif // ACTUATOR_H
