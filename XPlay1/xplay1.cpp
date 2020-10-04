#include "xplay1.h"
#include "XFFmpeg.h"
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include "XVideoThread.h"
#include "VideoWidget.h"

using std::cout;
using std::endl;

static bool isSliderPressed = false;

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

void XPlay1::sliderPress() {
    isSliderPressed = true;
}

void XPlay1::sliderRelease() {
    float pos = 0.0;
    int place = ui.progressSlider->value();

    isSliderPressed = false;
    pos = (float)place / (float)(ui.progressSlider->maximum() + 1);
    bool re = XFFmpeg::get()->seek(pos);
    if (re) {
        ui.progressSlider->setValue(place);
        cout << "[PLAY] Move slider to " << place << endl;
    }
    else {
        cout << "[PLAY] ERR: seek failed!" << endl;
    }
    return;
}

void XPlay1::timerEvent(QTimerEvent* e) {
    int videoPts = 0;
    int place = 0;
    int min = 0;
    int sec = 0;
    char buf[24] = { 0 };

    videoPts = XFFmpeg::get()->get_current_video_pts();
    min = (videoPts / 1000) / 60;
    sec = (videoPts / 1000) % 60;

    sprintf(buf, "%03d:%02d", min, sec);
    ui.playTime->setText(buf);

    if (XFFmpeg::get()->get_duration_ms() > 0) {
        if (!isSliderPressed) {          
            place = videoPts * ui.progressSlider->maximum() / XFFmpeg::get()->get_duration_ms();
            ui.progressSlider->setValue(place);
            cout << "[PLAY] Set slider to " << place << ", video pts is "<< videoPts << endl;
        }        
    }
    else if (XVideoThread::isExit) {
        ui.progressSlider->setValue(ui.progressSlider->maximum());
    }
    else {
        cout << "[PLAY] WRN: Total duration is 0, Set slider to 0!" << endl;
        ui.progressSlider->setValue(0);
    }
}