#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <thread>
#include <QObject>
#include "common.h"
#include "end.h"


class MainWindow;

typedef enum {
    CTRL_STOP           = 1,
    CTRL_RECOVER        = 2,
    CTRL_PAUSE          = 3,
    CTRL_RESUME         = 4,
    CTRL_RESET          = 5,
    CTRL_PREPARE_STRIP  = 6,
    CTRL_STRIP          = 7,
} ControllerCmd;


class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    qint64 getElapsedTimeMs() const {return mElapsedTimer.elapsed();}
    bool dealCmd(uint32_t cmd, int32_t end = -1); //cmd 取值 ControllerCmd
    bool setParam(int32_t end, uint32_t key, void* data, uint32_t dataLen);
    bool getParam(int32_t end, uint32_t key, void* data, uint32_t dataLen);

private:
    bool query();
    void poll();
    void analyseData(QByteArray &data);
    void analyseJsonPacket(QJsonObject &data);

signals:

private:
    QElapsedTimer mElapsedTimer;
    bool mIsStop, mIsPause; //用于标记来自外部命令的状态
    uint32_t mCtrlCmd;      //记录外部发送的命令
    QTimer mQueryTimer;
    std::thread* mPoller;
    End *mEndSet[End_num];
    Link *mLinkSet[Link_num];
    MainWindow* mWin;
};

#endif // CONTROLLER_H
