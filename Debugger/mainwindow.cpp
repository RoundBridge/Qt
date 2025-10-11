#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    op = new Operate(this);
    op_joint = new operate_joint(this);
    st = new State(this);
    ctrl = new Controller(this);
    op->setController(ctrl);
    op_joint->setController(ctrl);

    st->hide();
    op_joint->hide();
}

MainWindow::~MainWindow()
{
    delete st;
    delete op_joint;
    delete op;
    delete ctrl;
    delete ui;
}
