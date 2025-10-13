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

private:
    Ui::operate_joint *ui;
    QWidget *mParent;
    Controller* mCtrl;
};

#endif // OPERATE_JOINT_H
