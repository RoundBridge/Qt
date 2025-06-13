#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    op = new Operate(this);
    st = new State(this);
    ctrl = new Controller(this);
    op->setController(ctrl);

    st->hide();
}

MainWindow::~MainWindow()
{
    delete st;
    delete op;
    delete ctrl;
    delete ui;
}
