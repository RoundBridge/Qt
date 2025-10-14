#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <thread>
#include <QObject>
#include "common.h"
#include "end.h"


class MainWindow;

typedef enum {
    CTRL_QUERY          = 0,
    CTRL_STOP           = 1,
    CTRL_RECOVER        = 2,
    CTRL_PAUSE          = 3,
    CTRL_RESUME         = 4,
    CTRL_RESET          = 5,
    CTRL_PREPARE_STRIP  = 6,
    CTRL_STRIP          = 7,

    CTRL_JOINT_ROTATE       = 21,
    CTRL_JOINT_ROTATE_STOP  = 22,
    CTRL_JOINT_PITCH        = 23,
    CTRL_JOINT_PITCH_STOP   = 24,

    CTRL_ATTITUDE_ROTATE       = 31,
    CTRL_ATTITUDE_ROTATE_STOP  = 32,
    CTRL_ATTITUDE_PITCH        = 33,
    CTRL_ATTITUDE_PITCH_STOP   = 34,
    CTRL_ATTITUDE_YAW          = 35,
    CTRL_ATTITUDE_YAW_STOP     = 36,
} ControllerCmd;


class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    qint64 getElapsedTimeMs() const {return mElapsedTimer.elapsed();}
    bool dealCmd(uint32_t cmd, const std::string& end); //cmd 取值 ControllerCmd
    bool setParam(const std::string& end, uint32_t key, void* data, uint32_t dataLen);
    bool getParam(const std::string& end, uint32_t key, void* data, uint32_t dataLen);
    void dispCmdInfo(const std::string& end, const QString& info);
    void dispCmdRespInfo(const std::string& end, const QString& info);

private:
    bool query();
    void poll();
    void analyseData(QByteArray &data);
    void analyseJsonPacket(QJsonObject &data);

signals:

private:
    EndFactory mEndCreator;
    QElapsedTimer mElapsedTimer;
    bool mIsStop, mIsPause; //用于标记来自外部命令的状态
    uint32_t mCtrlCmd;      //记录外部发送的命令
    QTimer mQueryTimer;
    std::thread* mPoller;
    std::map<std::string, std::unique_ptr<End>> mEndSet;
    Link *mLinkSet[Link_num];
    MainWindow* mWin;
};

#endif // CONTROLLER_H
