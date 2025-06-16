#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define CENTER_NAME         "center"
#define CENTER_PORT         6666

#define MAIN_ACTUATOR_NAME  "actuator"
#define MAIN_ACTUATOR_IP    "10.168.1.171"
#define MAIN_ACTUATOR_PORT  6666

typedef enum {
    End_actuator = 0,
    End_bypass_gripper,
    End_num
} EndSet;

typedef struct {
    uint32_t magic;  //魔术字，四字节，低地址到高地址分别是'Q''D''Z''N'
    uint32_t crc32;  //CRC32校验，校验范围从包头中的len字段开始直到包体结束，即不包括magic和crc32
    uint32_t len;    //报文总长度【包头(固定16字节)+包体】
    uint32_t type;   //消息类型代码, 1-Json类型，其他-Binary
} QDMessageHdr;

typedef enum {
    QD_MESSAGE_TYPE_JSON = 1,           //json类型
    QD_MESSAGE_TYPE_BINARY = 2,         //二进制类型，用于升级
    QD_MESSAGE_TYPE_LOG = 256,          //日志信息
} QDMessageType;

typedef enum {
    EXE_SUCCESS = 0,            //成功执行
    EXE_FAIL,                   //执行失败
    EXE_EXECUTING,              //执行中
    EXE_ABNORMAL,               //异常
    EXE_WAIT                    //等待执行
} ExecuteState;

typedef struct {
    int32_t motorStripCurrent; //mA
    int32_t motorClampCurrent; //mA
    int32_t motorKnifeCurrent; //mA
    int32_t voltageStrip;      //mV
    int32_t voltageActuator;   //mV
    //...
} ActuatorState;

typedef enum {
    ACTUATOR_STATE = 1,         //获取机头状态，ActuatorState类型
    BYPASS_GRIPPER_STATE = 2,   //获取引流线抓线器状态
    CONTINUE_STRIP = 3,         //获取/设置是否全速剥皮，bool类型
} ParamKeyOpt;

#endif // COMMON_H
