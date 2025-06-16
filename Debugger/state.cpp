#include "state.h"
#include "ui_state.h"
#include "state.h"
#include "mainwindow.h"

State::State(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::State)
{
    mParent = parent;
    ui->setupUi(this);
}

State::~State()
{
    delete ui;
}

void State::displayLog(const QString &text) {
    ui->log->appendPlainText(text);
}

void State::on_back_clicked()
{
    MainWindow* p = dynamic_cast<MainWindow*>(mParent);
    hide();
    p->getOperateInstance()->show();
}

void State::on_logClear_clicked()
{
    // ui->log->appendPlainText("测试日志");
    ui->log->clear();
}

void State::on_logGet_clicked()
{
    // todo...
}

