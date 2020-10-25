#include "VideoWidget.h"
#include <QPainter>
#include "XFFmpeg.h"
#include <iostream>
#include "stdio.h"
#include "XVideoThread.h"
#include "xplay1.h"

using std::cout;
using std::endl;
// 调试时保存解码出来的图片用
int count = 0;
FILE* fp = NULL;

VideoWidget::VideoWidget(QWidget* p) :QOpenGLWidget(p) {
	//setFixedSize(800, 600);  // 设置后最大化或者拉动边框也不会调整尺寸了
	startTimer(30);  // 开启30毫秒定时器(注意，播放完一个文件后定时器并没有关闭，所以可以继续播放另一个文件)
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

void VideoWidget::paintEvent(QPaintEvent* e) {	// 重载窗口绘制事件函数	
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
	this->update();
	//this->repaint();  // 这两种方式都可以
	/*
	repaint():被调用之后，立即执行重绘，因此repaint是最快的，紧急情况下需要立刻重绘的可以使用repaint()。
	但是容易引起Recursive repaint，特别是调用repaint的函数不能放到paintEvent中调用，否则会造成死循环。
	update():跟repaint()比较，update则更加有优越性。update()调用之后并不是立即重绘，而是将重绘事件放入
	主消息循环中，由main的eventloop来统一调度的(其实也是比较快的)。update在调用paintEvent之前，还做了很
	多优化，如果update被调用了很多次，最后这些update会合并到一个大的重绘事件加入到消息队列，最后只有这
	个大的update被执行一次。同时也避免了repaint()中所提到的死循环。因此，一般情况下，调用update就够了，
	跟repaint()比起来，update是推荐使用的。
	*/
}

VideoWidget:: ~VideoWidget() {

}