#include "xplay1.h"
#include "XFFmpeg.h"
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include "XVideoThread.h"
#include "VideoWidget.h"
#include "XAudioPlay.h"
#include "XAudioThread.h"
#include "XDistributeThread.h"

using std::cout;
using std::endl;

#define PAUSE "QPushButton:hover{border-image:url(:/XPlay1/res/pause_hot.png);}\nQPushButton:!hover{border-image:url(:/XPlay1/res/pause.png);}"
#define PLAY "QPushButton:hover{border-image:url(:/XPlay1/res/play_hot.png);}\nQPushButton:!hover{border-image:url(:/XPlay1/res/play.png);}"
static bool isSliderPressed = false;

QReadWriteLock XPlay1::rwlock;
bool XPlay1::bSeek = false;

void XPlay1::rlock() {
    rwlock.lockForRead();
}

void XPlay1::wlock() {
    rwlock.lockForWrite();
}

void XPlay1::unlock() {
    rwlock.unlock();
}

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
    int ret = 0;
    char buf[24] = { 0 };
    //char* url = "rtmp://58.200.131.2:1935/livetv/cctv13";
    QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开")); // 英文的话，第二个参数直接就是QString("Open File")
    if (name.isEmpty()) {
        return;
    }

    XDistributeThread::get()->lock();
    this->setWindowTitle(name);
    ret = XFFmpeg::get()->open(name.toLocal8Bit());
    //ret = XFFmpeg::get()->open(url);
    if (0 > ret) {
        XDistributeThread::get()->unlock();
        cout << "[XPLAY] Open " << name.toStdString().data() << " failed, " << XFFmpeg::get()->get_error(ret) << endl;
        QMessageBox::information(this, "ERROR: ", "open file failed");
        return;
    }
    else {
        XVideoThread::bReset = true;
        XAudioThread::bReset = true;
        while (!XVideoThread::isExit);
        XVideoThread::bReset = false;
        while (!XAudioThread::isExit);
        XAudioThread::bReset = false;
        cout << "[XPLAY] Reset XDistributeThread!" << endl;
        XDistributeThread::bReset = true;
        XDistributeThread::get()->unlock();
        // 让分发线程退出一下
        while (!XDistributeThread::isExit);
        // 重新开启分发线程
        XDistributeThread::isExit = false;
        XDistributeThread::bReset = false;
        /*
            需要等待大概10毫秒左右，否则前一个视频没放完就新开一个视频时会无法启动
            XDistributeThread，也即下面的代码XDistributeThread::get()->start()不生效
        */
        QThread::msleep(10);  
        XDistributeThread::get()->start();
    }

    XAudioPlay::get()->sampleRate = XFFmpeg::get()->sampleRate;
    XAudioPlay::get()->channel = XFFmpeg::get()->channel;
    XAudioPlay::get()->sampleSize = XFFmpeg::get() ->sampleSize;
    XAudioPlay::get()->start();

    totalMs = XFFmpeg::get()->get_duration_ms(XFFmpeg::get()->videoStream);
    minutes = (totalMs / 1000) / 60;
    seconds = (totalMs / 1000) % 60;
    sprintf(buf, "%03d:%02d", minutes, seconds);  // 分别以3位和2位显示分钟和秒数
    ui.totalTime->setText(buf);

    // 视频线程
    XVideoThread::isStart = true;
    XVideoThread::isExit = false;
    XVideoThread::get()->start();

    // 音频线程
    XAudioThread::isStart = true;
    XAudioThread::isExit = false;
    XAudioThread::get()->start();

    set_play_state(1);  // 进入播放状态
    ui.play->setStyleSheet(PAUSE);  // 在播放状态下显示暂停按钮

    return;
}

void XPlay1::play() {
    if (get_play_state() == 1) {  // 当前是播放状态，点击后进入暂停状态
        set_play_state(2);
        ui.play->setStyleSheet(PLAY);  // 在暂停状态下显示播放按钮
        XVideoThread::isStart = false;
        XAudioThread::isStart = false;
    }
    else if(get_play_state() == 2){  // 当前是暂停状态，点击后进入播放状态
        set_play_state(1);
        ui.play->setStyleSheet(PAUSE);  // 在播放状态下显示暂停按钮
        XVideoThread::isStart = true;
        XAudioThread::isStart = true;
    }
}

void XPlay1::sliderPress() {
    isSliderPressed = true;
}

void XPlay1::sliderRelease() {
    float pos = 0.0;
    int place = ui.progressSlider->value();

    XPlay1::bSeek = true;
    isSliderPressed = false;
    pos = (float)place / (float)(ui.progressSlider->maximum() + 1);
    /*
        如果不加这个锁，那么下面seek后有可能XDistributeThread立马执行，读出pkt放到队列里面，等到
        这里继续运行下去又把队列清空了，于是就可能产生花屏（因为按照AVSEEK_FLAG_FRAME方式seek后
        第一次放进去的是I帧）
    */
    XDistributeThread::get()->lock();
    XPlay1::wlock();
    bool re = XFFmpeg::get()->seek(pos);
    if (re) {
        XDistributeThread::get()->clear_packet_list(XDistributeThread::get()->get_video_list());
        XDistributeThread::get()->clear_packet_list(XDistributeThread::get()->get_audio_list());
        ui.progressSlider->setValue(place);
        cout << "[XPLAY] Move slider to " << place << endl;
    }
    else {
        cout << "[XPLAY] ERR: seek failed!" << endl;
    }
    XPlay1::unlock();
    XDistributeThread::get()->unlock();
    XPlay1::bSeek = false;
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

    videoPts = XFFmpeg::get()->get_current_video_pts(NULL);
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
            //if (XVideoThread::isStart) {
            //    cout << "[XPLAY] Set slider to " << place << ", videoPts/totalVms/maxSliderBar is " << videoPts << "/" << totalVms << "/" << maxSliderBar << endl;
            //}
        }
    }
    else if (XVideoThread::isExit) {
        ui.progressSlider->setValue(ui.progressSlider->maximum());
    }
    else {
        //cout << "[XPLAY] WRN: Total duration is 0, Set slider to 0!" << endl;
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

void XPlay1::closeEvent(QCloseEvent* e) {
    XVideoThread::isStart = false;
    XVideoThread::isExit = true;
    XAudioThread::isStart = false;
    XAudioThread::isExit = true;
    XDistributeThread::isExit = true;
    QWidget::closeEvent(e);
}