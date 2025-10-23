#ifndef OPERATE_JOINT_H
#define OPERATE_JOINT_H

#include <QWidget>
#include "controller.h"

namespace Ui {
class operate_joint;
}

class operate_joint : public QWidget
{
    Q_OBJECT

public:
    explicit operate_joint(QWidget *parent = nullptr);
    ~operate_joint();

    void setController(Controller* ctrl);

    void displayAttitudeCmdInfo(const QString &text);

    void displayJointCmdInfo(const QString &text);

    void displayAttitudeCmdRespInfo(const QString &text);

    void displayJointCmdRespInfo(const QString &text);

private slots:
    void on_back_clicked();

    void on_stopAttitude_clicked();

    void on_recoverAttitude_clicked();

    void on_pauseAttitude_clicked();

    void on_resumeAttitude_clicked();

    void on_resetAttitude_clicked();

    void on_rotateAttitudeStop_clicked();

    void on_swingAttitudeStop_clicked();

    void on_yawAttitudeStop_clicked();

    void on_rPattern1Attitude_clicked();

    void on_rPattern2Attitude_clicked();

    void on_rPattern3Attitude_clicked();

    void on_rAngleAttitude_editingFinished();

    void on_rotateAttitude_clicked();

    void on_pAngleAttitude_editingFinished();

    void on_rSpeedAttitude_v_editingFinished();

    void on_rCurrentAttitude_v_editingFinished();

    void on_swingAttitude_clicked();

    void on_pPattern1Attitude_clicked();

    void on_pPattern2Attitude_clicked();

    void on_pPattern3Attitude_clicked();

    void on_pSpeedAttitude_v_editingFinished();

    void on_pCurrentAttitude_v_editingFinished();

    void on_stopJoint_clicked();

    void on_recoverJoint_clicked();

    void on_pauseJoint_clicked();

    void on_resumeJoint_clicked();

    void on_resetJoint_clicked();

    void on_rPattern1Joint_clicked();

    void on_rPattern2Joint_clicked();

    void on_rPattern3Joint_clicked();

    void on_rAngleJoint_editingFinished();

    void on_rSpeedJoint_v_editingFinished();

    void on_rCurrentJoint_v_editingFinished();

    void on_rotateJont_clicked();

    void on_rotateJointStop_clicked();

    void on_pPattern1Joint_clicked();

    void on_pPattern2Joint_clicked();

    void on_pPattern3Joint_clicked();

    void on_pAngleJoint_editingFinished();

    void on_pSpeedJoint_v_editingFinished();

    void on_pCurrentJoint_v_editingFinished();

    void on_swingJoint_clicked();

    void on_swingJointStop_clicked();

    void on_yPattern1Attitude_clicked();

    void on_yPattern2Attitude_clicked();

    void on_yPattern3Attitude_clicked();

    void on_yAngleAttitude_editingFinished();

    void on_ySpeedAttitude_v_editingFinished();

    void on_yCurrentAttitude_v_editingFinished();

    void on_yawAttitude_clicked();

private:
    Ui::operate_joint *ui;
    QWidget *mParent;
    Controller* mCtrl;
};

#endif // OPERATE_JOINT_H
