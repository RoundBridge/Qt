#include <QDebug>
#include <QIODevice>
#include "operate_joint.h"
#include "mainwindow.h"
#include "ui_operate_joint.h"

operate_joint::operate_joint(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::operate_joint)
{
    mParent = parent;
    ui->setupUi(this);
    ui->stopAttitude->setStyleSheet(
        "QPushButton {"
        "   background-color: rgb(220, 0, 0);"
        "   color: white;"              // 白色文字
        "   border: 2px solid #8B4513;" // 棕色边框
        "}"
        "QPushButton:hover {"
        "   background-color: rgb(255, 0, 0);" // 悬停时
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(200, 0, 0);"
        "    border-style: inset;"
        "}"
        );

    ui->pauseAttitude->setStyleSheet(
        "QPushButton {"
        "   background-color: rgb(220, 100, 0);"
        "   color: white;"              // 白色文字
        "   border: 2px solid #8B4513;" // 棕色边框
        "}"
        "QPushButton:hover {"
        "   background-color: rgb(255, 100, 0);" // 悬停时
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(200, 100, 0);"
        "    border-style: inset;"
        "}"
        );

    ui->stopJoint->setStyleSheet(
        "QPushButton {"
        "   background-color: rgb(220, 0, 0);"
        "   color: white;"              // 白色文字
        "   border: 2px solid #8B4513;" // 棕色边框
        "}"
        "QPushButton:hover {"
        "   background-color: rgb(255, 0, 0);" // 悬停时
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(200, 0, 0);"
        "    border-style: inset;"
        "}"
        );

    ui->pauseJoint->setStyleSheet(
        "QPushButton {"
        "   background-color: rgb(220, 100, 0);"
        "   color: white;"              // 白色文字
        "   border: 2px solid #8B4513;" // 棕色边框
        "}"
        "QPushButton:hover {"
        "   background-color: rgb(255, 100, 0);" // 悬停时
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(200, 100, 0);"
        "    border-style: inset;"
        "}"
        );

    // 设置 QPlainTextEdit 的右键菜单策略为自定义上下文菜单，便于触发 customContextMenuRequested 信号
    ui->aCmdInfo->setContextMenuPolicy(Qt::CustomContextMenu);
}

void operate_joint::displayAttitudeCmdInfo(const QString &text) {
    ui->aCmdInfo->appendPlainText(text);
}

void operate_joint::displayAttitudeCmdRespInfo(const QString &text) {
    ui->aCmdRespInfo->appendPlainText(text);
}

void operate_joint::displayJointCmdInfo(const QString &text) {
    ui->pCmdInfo->appendPlainText(text);
}

void operate_joint::displayJointCmdRespInfo(const QString &text) {
    ui->pCmdRespInfo->appendPlainText(text);
}

operate_joint::~operate_joint()
{
    delete ui;
}

void operate_joint::setController(Controller* ctrl) {
    mCtrl = ctrl;
}

void operate_joint::on_back_clicked()
{
    MainWindow* p = dynamic_cast<MainWindow*>(mParent);
    hide();
    p->getOperateInstance()->show();
}

void operate_joint::on_stopAttitude_clicked()
{
    qDebug() << "stop attitude clicked";
    mCtrl->dealCmd(CTRL_STOP, ATTITUDE_NAME);
}

void operate_joint::on_recoverAttitude_clicked()
{
    qDebug() << "recover attitude clicked";
    mCtrl->dealCmd(CTRL_RECOVER, ATTITUDE_NAME);
}

void operate_joint::on_pauseAttitude_clicked()
{
    qDebug() << "pause attitude clicked";
    mCtrl->dealCmd(CTRL_PAUSE, ATTITUDE_NAME);
}

void operate_joint::on_resumeAttitude_clicked()
{
    qDebug() << "resume attitude clicked";
    mCtrl->dealCmd(CTRL_RESUME, ATTITUDE_NAME);
}

void operate_joint::on_resetAttitude_clicked()
{
    qDebug() << "reset attitude clicked";
    mCtrl->dealCmd(CTRL_RESET, ATTITUDE_NAME);
}

void operate_joint::on_rotateAttitudeStop_clicked()
{
    qDebug() << "stop attitude rotate clicked";
    mCtrl->dealCmd(CTRL_ATTITUDE_ROTATE_STOP, ATTITUDE_NAME);
}

void operate_joint::on_swingAttitudeStop_clicked()
{
    qDebug() << "stop attitude swing clicked";
    mCtrl->dealCmd(CTRL_ATTITUDE_PITCH_STOP, ATTITUDE_NAME);
}

void operate_joint::on_yawAttitudeStop_clicked()
{
    qDebug() << "stop attitude yaw clicked";
    mCtrl->dealCmd(CTRL_ATTITUDE_YAW_STOP, ATTITUDE_NAME);
}

void operate_joint::on_rPattern1Attitude_clicked()
{
    int32_t v = 1;
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_ROTATE_PATTERN, &v, sizeof(v));
}

void operate_joint::on_rPattern2Attitude_clicked()
{
    int32_t v = 2;
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_ROTATE_PATTERN, &v, sizeof(v));
}

void operate_joint::on_rPattern3Attitude_clicked()
{
    int32_t v = 3;
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_ROTATE_PATTERN, &v, sizeof(v));
}

void operate_joint::on_rAngleAttitude_editingFinished()
{
    bool ok;
    float ang = ui->rAngleAttitude->text().toFloat(&ok);

    if (!ok) {
        ang = 0;
        qDebug() << "target rotate angle:" << ui->rAngleAttitude->text() << " error, set to default 0";
    }
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_ROTATE_ANGLE, &ang, sizeof(ang));
}

void operate_joint::on_rSpeedAttitude_v_editingFinished()
{
    bool ok;
    float spd = ui->rSpeedAttitude_v->text().toFloat(&ok);

    if (!ok) {
        spd = -1;
        qDebug() << "target rotate speed:" << ui->rSpeedAttitude_v->text() << " error, set to default -1";
    }
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_ROTATE_SPEED, &spd, sizeof(spd));
}

void operate_joint::on_rCurrentAttitude_v_editingFinished()
{
    bool ok;
    float cur = ui->rCurrentAttitude_v->text().toFloat(&ok);

    if (!ok) {
        cur = -1;
        qDebug() << "target rotate current:" << ui->rCurrentAttitude_v->text() << " error, set to default -1";
    }
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_ROTATE_CURRENT, &cur, sizeof(cur));
}

void operate_joint::on_rotateAttitude_clicked()
{
    qDebug() << "attitude rotate clicked";
    mCtrl->dealCmd(CTRL_ATTITUDE_ROTATE, ATTITUDE_NAME);
}

void operate_joint::on_pAngleAttitude_editingFinished()
{
    bool ok;
    float ang = ui->pAngleAttitude->text().toFloat(&ok);

    if (!ok) {
        ang = 0;
        qDebug() << "target swing angle:" << ui->pAngleAttitude->text() << " error, set to default 0";
    }
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_PITCH_ANGLE, &ang, sizeof(ang));
}

void operate_joint::on_pPattern1Attitude_clicked()
{
    int32_t v = 1;
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_PITCH_PATTERN, &v, sizeof(v));
}

void operate_joint::on_pPattern2Attitude_clicked()
{
    int32_t v = 2;
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_PITCH_PATTERN, &v, sizeof(v));
}

void operate_joint::on_pPattern3Attitude_clicked()
{
    int32_t v = 3;
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_PITCH_PATTERN, &v, sizeof(v));
}

void operate_joint::on_pSpeedAttitude_v_editingFinished()
{
    bool ok;
    float spd = ui->pSpeedAttitude_v->text().toFloat(&ok);

    if (!ok) {
        spd = -1;
        qDebug() << "target swing speed:" << ui->pSpeedAttitude_v->text() << " error, set to default -1";
    }
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_PITCH_SPEED, &spd, sizeof(spd));
}

void operate_joint::on_pCurrentAttitude_v_editingFinished()
{
    bool ok;
    float cur = ui->pCurrentAttitude_v->text().toFloat(&ok);

    if (!ok) {
        cur = -1;
        qDebug() << "target swing current:" << ui->pCurrentAttitude_v->text() << " error, set to default -1";
    }
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_PITCH_CURRENT, &cur, sizeof(cur));
}

void operate_joint::on_swingAttitude_clicked()
{
    qDebug() << "attitude swing clicked";
    mCtrl->dealCmd(CTRL_ATTITUDE_PITCH, ATTITUDE_NAME);
}

void operate_joint::on_stopJoint_clicked()
{
    qDebug() << "stop joint clicked";
    mCtrl->dealCmd(CTRL_STOP, JOINT_NAME);
}

void operate_joint::on_recoverJoint_clicked()
{
    qDebug() << "recover joint clicked";
    mCtrl->dealCmd(CTRL_RECOVER, JOINT_NAME);
}

void operate_joint::on_pauseJoint_clicked()
{
    qDebug() << "pause joint clicked";
    mCtrl->dealCmd(CTRL_PAUSE, JOINT_NAME);
}

void operate_joint::on_resumeJoint_clicked()
{
    qDebug() << "resume joint clicked";
    mCtrl->dealCmd(CTRL_RESUME, JOINT_NAME);
}

void operate_joint::on_resetJoint_clicked()
{
    qDebug() << "reset joint clicked";
    mCtrl->dealCmd(CTRL_RESET, JOINT_NAME);
}

void operate_joint::on_rPattern1Joint_clicked()
{
    int32_t v = 1;
    mCtrl->setParam(JOINT_NAME, SET_JOINT_ROTATE_PATTERN, &v, sizeof(v));
}

void operate_joint::on_rPattern2Joint_clicked()
{
    int32_t v = 2;
    mCtrl->setParam(JOINT_NAME, SET_JOINT_ROTATE_PATTERN, &v, sizeof(v));
}

void operate_joint::on_rPattern3Joint_clicked()
{
    int32_t v = 3;
    mCtrl->setParam(JOINT_NAME, SET_JOINT_ROTATE_PATTERN, &v, sizeof(v));
}

void operate_joint::on_rAngleJoint_editingFinished()
{
    bool ok;
    float ang = ui->rAngleJoint->text().toFloat(&ok);

    if (!ok) {
        ang = 0;
        qDebug() << "target rotate angle:" << ui->rAngleJoint->text() << " error, set to default 0";
    }
    mCtrl->setParam(JOINT_NAME, SET_JOINT_ROTATE_ANGLE, &ang, sizeof(ang));
}

void operate_joint::on_rSpeedJoint_v_editingFinished()
{
    bool ok;
    float spd = ui->rSpeedJoint_v->text().toFloat(&ok);

    if (!ok) {
        spd = -1;
        qDebug() << "target rotate speed:" << ui->rSpeedJoint_v->text() << " error, set to default -1";
    }
    mCtrl->setParam(JOINT_NAME, SET_JOINT_ROTATE_SPEED, &spd, sizeof(spd));
}

void operate_joint::on_rCurrentJoint_v_editingFinished()
{
    bool ok;
    float cur = ui->rCurrentJoint_v->text().toFloat(&ok);

    if (!ok) {
        cur = -1;
        qDebug() << "target rotate current:" << ui->rCurrentJoint_v->text() << " error, set to default -1";
    }
    mCtrl->setParam(JOINT_NAME, SET_JOINT_ROTATE_CURRENT, &cur, sizeof(cur));
}

void operate_joint::on_rotateJont_clicked()
{
    qDebug() << "joint rotate clicked";
    mCtrl->dealCmd(CTRL_JOINT_ROTATE, JOINT_NAME);
}

void operate_joint::on_rotateJointStop_clicked()
{
    qDebug() << "stop joint rotate clicked";
    mCtrl->dealCmd(CTRL_JOINT_ROTATE_STOP, JOINT_NAME);
}

void operate_joint::on_pPattern1Joint_clicked()
{
    int32_t v = 1;
    mCtrl->setParam(JOINT_NAME, SET_JOINT_PITCH_PATTERN, &v, sizeof(v));
}

void operate_joint::on_pPattern2Joint_clicked()
{
    int32_t v = 2;
    mCtrl->setParam(JOINT_NAME, SET_JOINT_PITCH_PATTERN, &v, sizeof(v));
}

void operate_joint::on_pPattern3Joint_clicked()
{
    int32_t v = 3;
    mCtrl->setParam(JOINT_NAME, SET_JOINT_PITCH_PATTERN, &v, sizeof(v));
}

void operate_joint::on_pAngleJoint_editingFinished()
{
    bool ok;
    float ang = ui->pAngleJoint->text().toFloat(&ok);

    if (!ok) {
        ang = 0;
        qDebug() << "target pitch angle:" << ui->pAngleJoint->text() << " error, set to default 0";
    }
    mCtrl->setParam(JOINT_NAME, SET_JOINT_PITCH_ANGLE, &ang, sizeof(ang));
}

void operate_joint::on_pSpeedJoint_v_editingFinished()
{
    bool ok;
    float spd = ui->pSpeedJoint_v->text().toFloat(&ok);

    if (!ok) {
        spd = -1;
        qDebug() << "target pitch speed:" << ui->pSpeedJoint_v->text() << " error, set to default -1";
    }
    mCtrl->setParam(JOINT_NAME, SET_JOINT_PITCH_SPEED, &spd, sizeof(spd));
}

void operate_joint::on_pCurrentJoint_v_editingFinished()
{
    bool ok;
    float cur = ui->pCurrentJoint_v->text().toFloat(&ok);

    if (!ok) {
        cur = -1;
        qDebug() << "target pitch current:" << ui->pCurrentJoint_v->text() << " error, set to default -1";
    }
    mCtrl->setParam(JOINT_NAME, SET_JOINT_PITCH_CURRENT, &cur, sizeof(cur));
}

void operate_joint::on_swingJoint_clicked()
{
    qDebug() << "joint pitch clicked";
    mCtrl->dealCmd(CTRL_JOINT_PITCH, JOINT_NAME);
}

void operate_joint::on_swingJointStop_clicked()
{
    qDebug() << "stop joint pitch clicked";
    mCtrl->dealCmd(CTRL_JOINT_PITCH_STOP, JOINT_NAME);
}

void operate_joint::on_yPattern1Attitude_clicked()
{
    int32_t v = 1;
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_YAW_PATTERN, &v, sizeof(v));
}

void operate_joint::on_yPattern2Attitude_clicked()
{
    int32_t v = 2;
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_YAW_PATTERN, &v, sizeof(v));
}

void operate_joint::on_yPattern3Attitude_clicked()
{
    int32_t v = 3;
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_YAW_PATTERN, &v, sizeof(v));
}

void operate_joint::on_yAngleAttitude_editingFinished()
{
    bool ok;
    float ang = ui->yAngleAttitude->text().toFloat(&ok);

    if (!ok) {
        ang = 0;
        qDebug() << "target yaw angle:" << ui->yAngleAttitude->text() << " error, set to default 0";
    }
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_YAW_ANGLE, &ang, sizeof(ang));
}

void operate_joint::on_ySpeedAttitude_v_editingFinished()
{
    bool ok;
    float spd = ui->ySpeedAttitude_v->text().toFloat(&ok);

    if (!ok) {
        spd = -1;
        qDebug() << "target yaw speed:" << ui->ySpeedAttitude_v->text() << " error, set to default -1";
    }
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_YAW_SPEED, &spd, sizeof(spd));
}

void operate_joint::on_yCurrentAttitude_v_editingFinished()
{
    bool ok;
    float cur = ui->yCurrentAttitude_v->text().toFloat(&ok);

    if (!ok) {
        cur = -1;
        qDebug() << "target yaw current:" << ui->yCurrentAttitude_v->text() << " error, set to default -1";
    }
    mCtrl->setParam(ATTITUDE_NAME, SET_ATTITUDE_YAW_CURRENT, &cur, sizeof(cur));
}

void operate_joint::on_yawAttitude_clicked()
{
    qDebug() << "attitude yaw clicked";
    mCtrl->dealCmd(CTRL_ATTITUDE_YAW, ATTITUDE_NAME);
}

void operate_joint::on_aCmdInfo_customContextMenuRequested(const QPoint &pos)
{
    qDebug() << "mouse point (" << pos.x() << ", " << pos.y() << ")";
    ui->aCmdInfo->clear(); //清空内容
    // 更详细的可参考：https://www.cnblogs.com/acmexyz/p/11551565.html
}

