#ifndef OPERATE_H
#define OPERATE_H

#include <QWidget>
#include "controller.h"

namespace Ui {
class Operate;
}

class Operate : public QWidget
{
    Q_OBJECT

public:
    explicit Operate(QWidget *parent = nullptr);
    ~Operate();

    void setController(Controller* ctrl);

private slots:
    // void doQuery();

    void on_reset_clicked();

    void on_contentIP_editingFinished();

    void on_contentPort_editingFinished();

    void on_stop_clicked();

    void on_recover_clicked();

    void on_pause_clicked();

    void on_resume_clicked();

private:
    Ui::Operate *ui;
    Controller* mCtrl;
    // QUdpSocket *mSocket;
    // QTimer *mQueryTimer;
    // QHostAddress mReceiverAddress;
    // quint16 mReceiverPort;
    // uint32_t mCmd;
    // uint32_t mSeq;
};

#endif // OPERATE_H
