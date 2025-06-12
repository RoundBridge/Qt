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
    void updateEndState(bool valid, qint64 ms);

    virtual uint32_t getMappedCmd(uint32_t ctrlCmd);
    virtual bool processCmd(uint32_t ctrlCmd);
    virtual bool reConnect();
    virtual bool query();

    int mEndId;
    int mConnectState;
    qint64 mHeartBeatMs;
    qint64 mLastReconnectMs;
    Controller* mCtrl;
};

#endif // END_H
