#ifndef END_H
#define END_H

#include <QObject>
#include <functional>
#include <map>
#include "link.h"

class End;
class Controller; //前置声明

class EndWorker : public QObject {
    Q_OBJECT
public:
    explicit EndWorker(End* e, QObject* parent = nullptr) : QObject(parent) {mEnd = e;}
    void stopWork();

public slots:
    void doWork();

signals:
    void workFinished(); // 任务结束信号

private:
    const int32_t mWorkSleepGapMs = 200;
    End* mEnd;
    bool mStop = false; // 停止标志（需线程安全访问?）
    // QMutex mMutex; // 保护mStop的互斥锁
};

class End
{
public:
    End(Controller* controller, const char* remoteIp, quint16 remotePort, Link* link, const std::string& id = "null end");
    virtual ~End();

    bool execute(uint32_t ctrlCmd);
    bool isConnect() const {return mConnectState == Link_Connect;}
    void updateEndConnectState(bool valid, qint64 ms);

    virtual uint32_t makeCmd(uint32_t c, uint32_t s, int32_t msgType, QByteArray& out, QJsonObject* extra=nullptr, QByteArray* byte=nullptr);
    virtual void updateEndExeState(uint32_t endCmd, uint32_t seq, uint32_t state);
    virtual void parseExtraInfo(uint32_t endCmd, QJsonObject &e);
    virtual bool setParam(uint32_t key, void* data, uint32_t dataLen);
    virtual bool getParam(uint32_t key, void* data, uint32_t dataLen);
    virtual uint32_t getMappedCmd(uint32_t ctrlCmd);
    virtual bool processCmd(uint32_t ctrlCmd);
    virtual bool doWorkProc(bool stop);
    virtual bool reConnect();
    virtual bool query();
    virtual bool stop();
    virtual bool recover();
    virtual bool pause();
    virtual bool resume();
    virtual bool reset();

    std::string mEndName;
    // const std::string& mEndName; //为什么用这种方式，mEndName在后续使用时会变空，比如在End::execute中打印时就是空的
    int mConnectState;

    uint32_t mCmd, mSeq;
    uint32_t mRodState;
    quint16 mRemotePort;
    QHostAddress mRemoteIp;

    uint32_t mExeState;     //命令执行状态
    qint64 mHeartBeatMs;
    qint64 mLastReconnectMs;
    Controller* mCtrl;
    Link* mLink; //信令连接，公用连接，如果以后一个末端可以有多个连接，甚至连接的类型还可能不一样，那么差异化的连接放到子类里
    EndWorker* mWorker;
};

class EndFactory {
private:
    using CreateMethod = std::function<std::unique_ptr<End>(Controller* c, Link* link)>;
    std::map<std::string, CreateMethod> creators;

public:
    EndFactory();

    void registerCreator(const std::string& type, CreateMethod creator) {
        creators[type] = creator;
    }

    std::unique_ptr<End> create(const std::string& type, Controller* controller, Link* link);
};

#endif // END_H
