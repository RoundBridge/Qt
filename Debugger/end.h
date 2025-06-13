#ifndef END_H
#define END_H

#include "link.h"

class Controller; //前置声明

class End
{
public:    
    End(Controller* controller, int id = -1);
    virtual ~End();

    static End* createEnd(Controller* controller, Link* link, int id = -1);
    static void destroyEnd(End* e);

    bool execute(uint32_t ctrlCmd);
    int getId() const {return mEndId;}
    bool isConnect() const {return mConnectState == Link_Connect;}
    void updateEndConnectState(bool valid, qint64 ms);

    virtual void updateEndExeState(uint32_t endCmd, uint32_t seq, uint32_t state);
    virtual void parseExtraInfo(uint32_t endCmd, QJsonObject &e);
    virtual bool setParam(uint32_t key, void* data, uint32_t dataLen);
    virtual bool getParam(uint32_t key, void* data, uint32_t dataLen);
    virtual uint32_t getMappedCmd(uint32_t ctrlCmd);
    virtual bool processCmd(uint32_t ctrlCmd);
    virtual bool reConnect();
    virtual bool query();

    int mEndId;
    int mConnectState;
    uint32_t mExeState;     //命令执行状态
    qint64 mHeartBeatMs;
    qint64 mLastReconnectMs;
    Controller* mCtrl;
};

#endif // END_H
