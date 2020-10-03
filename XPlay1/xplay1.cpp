#include "xplay1.h"
#include "XFFmpeg.h"
#include <QFileDialog>
#include <QMessageBox>
#include "XVideoThread.h"
#include "VideoWidget.h"

XPlay1::XPlay1(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

void XPlay1::open() {
    int totalMs = 0, minutes = 0, seconds = 0;
    char buf[24] = { 0 };

    QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开")); // 英文的话，第二个参数直接就是QString("Open File")
    if (name.isEmpty()) {
        return;
    }
    this->setWindowTitle(name);
    if (0 != XFFmpeg::get()->open(name.toLocal8Bit())) {
        QMessageBox::information(this, "ERROR: ", "open file failed");
        return;
    }

    totalMs = XFFmpeg::get()->get_duration_ms();
    minutes = (totalMs / 1000) / 60;
    seconds = (totalMs / 1000) % 60;
    sprintf(buf, "00:00/%d:%d", minutes, seconds);
    ui.playTime->setText(buf);

    XVideoThread::isStart = true;
    XVideoThread::isExit = false;

    XVideoThread::get()->start();

    return;
}