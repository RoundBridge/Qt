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
    void on_reset_clicked();

    void on_contentIP_editingFinished();

    void on_contentPort_editingFinished();

    void on_stop_clicked();

    void on_recover_clicked();

    void on_pause_clicked();

    void on_resume_clicked();

    void on_prepareStrip_clicked();

    void on_ActuatorState_clicked();

    void on_strip_clicked();

    void on_continueStrip_clicked(bool checked);

private:
    QWidget *mParent;
    Ui::Operate *ui;
    Controller* mCtrl;
};

#endif // OPERATE_H
