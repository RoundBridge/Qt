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
    startTimer(40);  // 40毫秒刷新一次当前播放时间
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
    sprintf(buf, "%03d:%02d", minutes, seconds);  // 分别以3位和2位显示分钟和秒数
    ui.totalTime->setText(buf);

    XVideoThread::isStart = true;
    XVideoThread::isExit = false;
    XVideoThread::get()->start();    
    return;
}

void XPlay1::timerEvent(QTimerEvent* e) {
    int min = (XFFmpeg::get()->get_current_video_pts() / 1000) / 60;
    int sec = (XFFmpeg::get()->get_current_video_pts() / 1000) % 60;
    char buf[24] = { 0 };
    sprintf(buf, "%03d:%02d", min, sec);
    ui.playTime->setText(buf);

    if (XFFmpeg::get()->get_duration_ms() > 0) {
        ui.progressSlider->setValue(XFFmpeg::get()->get_current_video_pts() * 999 / XFFmpeg::get()->get_duration_ms());
    }
    else {
        ui.progressSlider->setValue(0);
    }
}