#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "operate.h"
#include "operate_joint.h"
#include "state.h"
#include "controller.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    State* getStateInstance() const {return st;}
    Operate* getOperateInstance() const {return op;}
    operate_joint* getJointOperateInstance() const {return op_joint;}

private:
    Ui::MainWindow *ui;
    Operate *op;
    operate_joint *op_joint;
    State *st;
    Controller *ctrl;
};
#endif // MAINWINDOW_H
