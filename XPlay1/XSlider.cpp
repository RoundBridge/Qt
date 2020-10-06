#include "XSlider.h"
#include <QMouseEvent>

XSlider::XSlider(QWidget* p) :QSlider(p) {

}

XSlider::~XSlider() {

}

void XSlider::mousePressEvent(QMouseEvent* e) {
	int val = 0;
	val = (this->maximum()+1) * e->pos().x() / this->width();
	this->setValue(val);
	QSlider::mousePressEvent(e);
	return;
}
