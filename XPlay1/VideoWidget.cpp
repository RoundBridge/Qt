#include "VideoWidget.h"
#include <QPainter>
#include "XFFmpeg.h"
#include <iostream>
#include "stdio.h"
#include "XVideoThread.h"

using std::cout;
using std::endl;
// 调试时保存解码出来的图片用
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

	startTimer(30);  // 开启30毫秒定时器(注意，播放完一个文件后定时器并没有关闭，所以可以继续播放另一个文件)
}

void VideoWidget::paintEvent(QPaintEvent* e) {	// 重载窗口绘制事件函数	
	QPainter painter;

	if (XVideoThread::isExit) {
		return;
	}
	XFFmpeg::get()->video_convert(image->bits(), width(), height(), AV_PIX_FMT_BGRA);
	// 调试用（保存解码出来的第100帧，看数据是否正确）
	if (count == 100) {
		//fp = fopen("argb.data", "wb");
		//fwrite(image->bits(), width()*height()*4, 1, fp);
		//fclose(fp); /*关闭文件*/
	}
	// 开始画之前先清理屏幕
	painter.begin(this);
	// 绘制image
	painter.drawImage(QPoint(0, 0), *image);
	// 最后将画面绘制上去
	painter.end();

	count++;
}

void VideoWidget::timerEvent(QTimerEvent* e) {		// 重载定时器事件函数（界面刷新通过定时器进行）
	//this->update();
	this->repaint();  // 这两种方式都可以
}

VideoWidget:: ~VideoWidget() {

}