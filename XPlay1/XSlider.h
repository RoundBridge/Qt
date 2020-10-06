#pragma once
#include <QSlider> 

class XSlider:public QSlider
{
public:
	XSlider(QWidget* p = NULL);
	virtual ~XSlider();
	void mousePressEvent(QMouseEvent *e);
};

