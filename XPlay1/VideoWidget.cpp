#include "VideoWidget.h"
#include <QPainter>
#include "XFFmpeg.h"
#include <iostream>
#include "stdio.h"
#include "XVideoThread.h"
#include "xplay1.h"

using std::cout;
using std::endl;
// ����ʱ������������ͼƬ��
int count = 0;
FILE* fp = NULL;

VideoWidget::VideoWidget(QWidget* p) :QOpenGLWidget(p) {
	//setFixedSize(800, 600);  // ���ú���󻯻��������߿�Ҳ��������ߴ���
	startTimer(30);  // ����30���붨ʱ��(ע�⣬������һ���ļ���ʱ����û�йرգ����Կ��Լ���������һ���ļ�)
}

bool VideoWidget::resize_image_buffer() {
	static int w = -1;
	static int h = -1;

	if (w != width() || h != height()) {
		cout << "VideoWidget w/h: " << width() << "/" << height() << endl;
		cout << "Last time w/h: " << w << "/" << h << endl;

		w = width();
		h = height();
		if (image) {
			delete image->bits();
			delete image;
			image = NULL;
		}
		cout << "Alloc memory for image!" << endl;
		uchar* buf = new uchar[width() * height() * 4];
		image = new QImage(buf, width(), height(), QImage::Format_ARGB32);
		if (image == NULL) {
			cout << "new image buf failed" << endl;
			return false;
		}
		return true;
	}
	else {
		return true;
	}
}

void VideoWidget::paintEvent(QPaintEvent* e) {	// ���ش��ڻ����¼�����	
	QPainter painter;

	if (XVideoThread::isExit || XPlay1::bSeek) {
		return;
	}
	if (true != resize_image_buffer()) {
		return;
	}
	if (true != XFFmpeg::get()->video_convert(image->bits(), width(), height(), AV_PIX_FMT_BGRA)) {
		return;
	}
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
	this->update();
	//this->repaint();  // �����ַ�ʽ������
	/*
	repaint():������֮������ִ���ػ棬���repaint�����ģ������������Ҫ�����ػ�Ŀ���ʹ��repaint()��
	������������Recursive repaint���ر��ǵ���repaint�ĺ������ܷŵ�paintEvent�е��ã�����������ѭ����
	update():��repaint()�Ƚϣ�update���������Խ�ԡ�update()����֮�󲢲��������ػ棬���ǽ��ػ��¼�����
	����Ϣѭ���У���main��eventloop��ͳһ���ȵ�(��ʵҲ�ǱȽϿ��)��update�ڵ���paintEvent֮ǰ�������˺�
	���Ż������update�������˺ܶ�Σ������Щupdate��ϲ���һ������ػ��¼����뵽��Ϣ���У����ֻ����
	�����update��ִ��һ�Ρ�ͬʱҲ������repaint()�����ᵽ����ѭ������ˣ�һ������£�����update�͹��ˣ�
	��repaint()��������update���Ƽ�ʹ�õġ�
	*/
}

VideoWidget:: ~VideoWidget() {

}