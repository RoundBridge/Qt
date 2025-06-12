#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    op = new Operate(this);
    ctrl = new Controller(this);
    op->setController(ctrl);
}

MainWindow::~MainWindow()
{
    delete op;
    delete ctrl;
    delete ui;
}
