#pragma once
#include <qwidget.h>
#include <QOpenGLWidget>

class VideoWidget:public QOpenGLWidget
{
public:
	VideoWidget(QWidget* p = NULL);
	virtual ~VideoWidget();
	void paintEvent(QPaintEvent *e);		// 重载窗口绘制事件函数
	void timerEvent(QTimerEvent* e);		// 重载定时器事件函数（界面刷新通过定时器进行）

private:
	QImage* image = NULL;
};

