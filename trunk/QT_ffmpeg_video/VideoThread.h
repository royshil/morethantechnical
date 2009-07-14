/*
 * VideoThread.h
 *
 *  Created on: Oct 30, 2008
 *      Author: root
 */

#ifndef VIDEOTHREAD_H_
#define VIDEOTHREAD_H_
#include "/usr/include/qt4/QtCore/qthread.h"
#include "VideoWidget.h"

class VideoThread : public QThread {
	Q_OBJECT
public:
	VideoThread(VideoWidget* v);
	virtual ~VideoThread();
	void stopPlaying();
private:
	VideoWidget* _v;
	void playVideo();
	void run();
	bool running;
signals:
	void updateWidget();
	void videoSetup(int lengthMsec);
};

#endif /* VIDEOTHREAD_H_ */
