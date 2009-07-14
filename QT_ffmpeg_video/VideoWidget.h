/*
 * VideoWidget.h
 *
 *  Created on: Oct 30, 2008
 *      Author: root
 */

#ifndef VIDEOWIDGET_H_
#define VIDEOWIDGET_H_
#include <QtGui/QWidget>

class VideoWidget : public QWidget {
	Q_OBJECT

public:
	VideoWidget(QWidget *parent = 0);
    ~VideoWidget();
    uchar* buf;
    int len;

private:
	void paintEvent(QPaintEvent *);
};

#endif /* VIDEOWIDGET_H_ */
