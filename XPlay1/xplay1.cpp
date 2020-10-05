#include "xplay1.h"
#include "XFFmpeg.h"
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include "XVideoThread.h"
#include "VideoWidget.h"

using std::cout;
using std::endl;

#define PAUSE "QPushButton:hover{border-image:url(:/XPlay1/res/pause_hot.png);}\nQPushButton:!hover{border-image:url(:/XPlay1/res/pause.png);}"
#define PLAY "QPushButton:hover{border-image:url(:/XPlay1/res/play_hot.png);}\nQPushButton:!hover{border-image:url(:/XPlay1/res/play.png);}"
static bool isSliderPressed = false;

XPlay1::XPlay1(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    startTimer(40);  // 40毫秒刷新一次当前播放时间
}

int XPlay1::get_play_state() {
    return playState;
}

void XPlay1::set_play_state(int state) {
    playState = state;
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

    totalMs = XFFmpeg::get()->get_duration_ms(XFFmpeg::get()->videoStream);
    minutes = (totalMs / 1000) / 60;
    seconds = (totalMs / 1000) % 60;
    sprintf(buf, "%03d:%02d", minutes, seconds);  // 分别以3位和2位显示分钟和秒数
    ui.totalTime->setText(buf);

    XVideoThread::isStart = true;
    XVideoThread::isExit = false;
    XVideoThread::get()->start();
    set_play_state(1);  // 进入播放状态
    ui.play->setStyleSheet(PAUSE);  // 在播放状态下显示暂停按钮

    return;
}

void XPlay1::play() {
    if (get_play_state() == 1) {  // 当前是播放状态，点击后进入暂停状态
        set_play_state(2);
        ui.play->setStyleSheet(PLAY);  // 在暂停状态下显示播放按钮
        XVideoThread::isStart = false;
    }
    else if(get_play_state() == 2){  // 当前是暂停状态，点击后进入播放状态
        set_play_state(1);
        ui.play->setStyleSheet(PAUSE);  // 在播放状态下显示暂停按钮
        XVideoThread::isStart = true;
    }
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
    int64_t  videoPts = 0;  // 用64位防溢出
    int place = 0;
    int min = 0;
    int sec = 0;
    int maxSliderBar = 0;
    int64_t  totalVms = 0;
    char buf[24] = { 0 };

    videoPts = XFFmpeg::get()->get_current_video_pts();
    min = (videoPts / 1000) / 60;
    sec = (videoPts / 1000) % 60;

    sprintf(buf, "%03d:%02d", min, sec);
    ui.playTime->setText(buf);

    if (XFFmpeg::get()->get_duration_ms(XFFmpeg::get()->videoStream) > 0) {
        if (!isSliderPressed) {
            maxSliderBar = ui.progressSlider->maximum();
            totalVms = XFFmpeg::get()->get_duration_ms(XFFmpeg::get()->videoStream);
            place = videoPts * maxSliderBar / totalVms;  // videoPts * maxSliderBar如果是32位类型变量，则对于大的视频会溢出
            ui.progressSlider->setValue(place);
            //if(XVideoThread::isStart)
                //cout << "[PLAY] Set slider to " << place << ", videoPts/totalVms/maxSliderBar is "<< videoPts << "/" << totalVms << "/" << maxSliderBar << endl;
        }
    }
    else if (XVideoThread::isExit) {
        ui.progressSlider->setValue(ui.progressSlider->maximum());
    }
    else {
        //cout << "[PLAY] WRN: Total duration is 0, Set slider to 0!" << endl;
        ui.progressSlider->setValue(0);
    }
}

void XPlay1::resizeEvent(QResizeEvent* e) {
    ui.openGLWidget->resize(size());  // size()获取当前窗口的大小（是XPlay1的窗口大小，而不是openGLWidget窗口的大小）
    ui.progressSlider->move(50, this->height() - 70);
    ui.progressSlider->resize(this->width() - 100, ui.progressSlider->height());
    ui.play->move(this->width() / 2 + 50, ui.progressSlider->y() + 17);
    ui.open->move(this->width() / 2 - 50, ui.progressSlider->y() + 11);
    ui.playTime->move(ui.progressSlider->x() + 20, ui.play->y() + 10);
    ui.separator->move(ui.playTime->x() + ui.playTime->width() - 11, ui.playTime->y()+2);
    ui.totalTime->move(ui.playTime->x() + ui.playTime->width() + 4, ui.playTime->y());
}   