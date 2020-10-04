#include "VideoWidget.h"
#include <QPainter>
#include "XFFmpeg.h"
#include <iostream>
#include "stdio.h"
#include "XVideoThread.h"

using std::cout;
using std::endl;
// ����ʱ������������ͼƬ��
int count = 0;
FILE* fp = NULL;

VideoWidget::VideoWidget(QWidget* p) :QOpenGLWidget(p) {
	int re = 0;
	setFixedSize(800, 600);
	cout << "VideoWidget w/h: " << width() << "/" << height() << endl;

	if (image == NULL) {
		cout << "Alloc memory for image!" << endl;
		uchar* buf = new uchar[width()*height()*4];
		image = new QImage(buf,width(),height(),QImage::Format_ARGB32);
		if (image == NULL) {
			cout << "new image buf failed" << endl;
			return;
		}
	}

	startTimer(30);  // ����30���붨ʱ��(ע�⣬������һ���ļ���ʱ����û�йرգ����Կ��Լ���������һ���ļ�)
}

void VideoWidget::paintEvent(QPaintEvent* e) {	// ���ش��ڻ����¼�����	
	QPainter painter;

	if (XVideoThread::isExit) {
		return;
	}
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
	this->repaint();  // �����ַ�ʽ������
}

VideoWidget:: ~VideoWidget() {

}