#pragma once
#include <qwidget.h>
#include <QOpenGLWidget>

class VideoWidget:public QOpenGLWidget
{
public:
	VideoWidget(QWidget* p = NULL);
	virtual ~VideoWidget();
	void paintEvent(QPaintEvent *e);		// ���ش��ڻ����¼�����
	void timerEvent(QTimerEvent* e);		// ���ض�ʱ���¼�����������ˢ��ͨ����ʱ�����У�

private:
	QImage* image = NULL;
};

