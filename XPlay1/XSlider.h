#pragma once
#include <QtWidgets/QWidget>
#include <QSlider> 

class XSlider:public QSlider
{
	Q_OBJECT
public:
	XSlider(QWidget* p = NULL);
	virtual ~XSlider();
	void mousePressEvent(QMouseEvent *e);
};

