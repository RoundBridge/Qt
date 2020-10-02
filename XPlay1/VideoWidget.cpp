#include "VideoWidget.h"
#include <QPainter>
#include "XFFmpeg.h"
#include <iostream>
#include "stdio.h"
#include "XVideoThread.h"

using std::cout;
using std::endl;

int count = 0;
FILE* fp = NULL;

VideoWidget::VideoWidget(QWidget* p) :QOpenGLWidget(p) {
	int re = 0;
	setFixedSize(800, 600);
	cout << "VideoWidget w/h: " << width() << "/" << height() << endl;

	if (image == NULL) {
		uchar* buf = new uchar[width()*height()*4];
		image = new QImage(buf,width(),height(),QImage::Format_ARGB32);
		if (image == NULL) {
			cout << "new image buf failed" << endl;
			return;
		}
	}
	re = XFFmpeg::get()->open("1024.mp4");
	if (re != 0) {
		cout << "open video failed" << endl;
		return;
	}

	startTimer(30);  // ����20���붨ʱ��
	XVideoThread::get()->start();
}

void VideoWidget::paintEvent(QPaintEvent* e) {	// ���ش��ڻ����¼�����	
	QPainter painter;
/*
	AVPacket *pkt = NULL;
	AVFrame* yuv = NULL;
	// ��ȡ��Ƶ������İ�
	pkt = XFFmpeg::get()->read();
	if (pkt == NULL) {
		cout << "A NULL PTR returned from read()" << endl;
		return;
	}

	if (pkt->stream_index != XFFmpeg::get()->videoStream) {
		return;
	}
	// �Զ�ȡ���İ����н���
	yuv = XFFmpeg::get()->decode(pkt);
	av_packet_unref(pkt);
	if (yuv == NULL) {
		return;
	}
*/
	XFFmpeg::get()->video_convert(image->bits(), width(), height(), AV_PIX_FMT_BGRA);
	// �����ã������������ĵ�100֡���������Ƿ���ȷ��
	if (count == 100) {
		//fp = fopen("argb.data", "wb");
		//fwrite(image->bits(), width()*height()*4, 1, fp);
		//fclose(fp); /*�ر��ļ�*/
	}
	// ��ʼ��֮ǰ��������Ļ
	painter.begin(this);
	// ����image
	painter.drawImage(QPoint(0, 0), *image);
	// ��󽫻��������ȥ
	painter.end();

	count++;
}

void VideoWidget::timerEvent(QTimerEvent* e) {		// ���ض�ʱ���¼�����������ˢ��ͨ����ʱ�����У�
	//this->update();
	//cout << "time out" << endl;
	this->repaint();
}

VideoWidget:: ~VideoWidget() {

}