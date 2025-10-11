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

private slots:
    void on_back_clicked();

private:
    Ui::operate_joint *ui;
    QWidget *mParent;
    Controller* mCtrl;
};

#endif // OPERATE_JOINT_H
