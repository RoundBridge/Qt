#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <thread>
#include <QObject>
#include <common.h>
#include "end.h"


typedef enum {
    CTRL_STOP           = 1,
    CTRL_RECOVER        = 2,
    CTRL_PAUSE          = 3,
    CTRL_RESUME         = 4,
    CTRL_RESET          = 5,
} ControllerCmd;


class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    qint64 getElapsedTimeMs() const {return mElapsedTimer.elapsed();}
    bool dealCmd(uint32_t cmd, int32_t end = -1); //cmd 取值 ControllerCmd

private:
    bool query();
    void poll();

signals:

private:
    QElapsedTimer mElapsedTimer;
    bool mIsStop, mIsPause; //用于标记来自外部命令的状态
    QTimer mQueryTimer;
    std::thread* mPoller;
    End *mEndSet[End_num];
    Link *mLinkSet[Link_num];
};

#endif // CONTROLLER_H
