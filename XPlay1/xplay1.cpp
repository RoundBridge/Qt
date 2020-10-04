#include "xplay1.h"
#include "XFFmpeg.h"
#include <QFileDialog>
#include <QMessageBox>
#include "XVideoThread.h"
#include "VideoWidget.h"

static bool isSliderPressed = false;

XPlay1::XPlay1(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    startTimer(40);  // 40����ˢ��һ�ε�ǰ����ʱ��
}

void XPlay1::open() {
    int totalMs = 0, minutes = 0, seconds = 0;
    char buf[24] = { 0 };

    QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("��")); // Ӣ�ĵĻ����ڶ�������ֱ�Ӿ���QString("Open File")
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
    sprintf(buf, "%03d:%02d", minutes, seconds);  // �ֱ���3λ��2λ��ʾ���Ӻ�����
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
    }
    return;
}

void XPlay1::timerEvent(QTimerEvent* e) {
    int min = (XFFmpeg::get()->get_current_video_pts() / 1000) / 60;
    int sec = (XFFmpeg::get()->get_current_video_pts() / 1000) % 60;
    char buf[24] = { 0 };
    sprintf(buf, "%03d:%02d", min, sec);
    ui.playTime->setText(buf);

    if (XFFmpeg::get()->get_duration_ms() > 0) {
        if (!isSliderPressed) {
            ui.progressSlider->setValue(
                XFFmpeg::get()->get_current_video_pts() 
                * ui.progressSlider->maximum() 
                / XFFmpeg::get()->get_duration_ms());
        }        
    }
    else {
        ui.progressSlider->setValue(0);
    }
}