#include "VideoWidget.h"
#include "/usr/include/qt4/QtGui/qpainter.h"
#include "/usr/include/qt4/QtGui/qimage.h"

VideoWidget::VideoWidget(QWidget* parent) :
	QWidget(parent),buf(NULL),len(0)
{

}

VideoWidget::~VideoWidget() {

}

void VideoWidget::paintEvent(QPaintEvent * e) {
	QPainter painter(this);
//	painter.drawRect(rect());

	if(buf) {
		QImage i = QImage::fromData(buf,len,"PPM");
		painter.drawImage(QPoint(0,0),i);
	}
}
