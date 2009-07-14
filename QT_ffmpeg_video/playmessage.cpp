#include "playmessage.h"
#include "/usr/include/qt4/QtGui/qsound.h"
#include "/usr/include/qt4/QtGui/qmessagebox.h"
#include "/usr/include/qt4/QtCore/qtimer.h"
#include "/usr/include/qt4/QtGui/qimage.h"
#include "VideoWidget.h"
#include "/usr/include/qt4/QtGui/qpainter.h"
#include "/usr/include/qt4/QtCore/qthread.h"
#include "/usr/include/qt4/QtCore/qwaitcondition.h"
#include "VideoThread.h"
#include "sdl_play_sound.h"

playMessage::playMessage(QWidget *parent)
    : QWidget(parent),_timer(NULL)
{
	ui.setupUi(this);
	v = new VideoWidget(this);
	v->setMinimumWidth(160);
	v->setMinimumHeight(120);
	v->setVisible(true);
	ui.frame_2->layout()->addWidget(v);

	connect(ui.toolButtonBack,SIGNAL(clicked(bool)),this,SLOT(backClicked(bool)));
	connect(ui.toolButtonPlay,SIGNAL(clicked(bool)),this,SLOT(playClicked(bool)));
	connect(ui.toolButtonStop,SIGNAL(clicked(bool)),this,SLOT(stopClicked(bool)));

	///TODO: implement all the buttons

	ui.progressBar->setFormat("%v");
	ui.progressBar->setMaximum(7000);

    InitSound("girl.wav");

	QTimer::singleShot(0,this,SLOT(doPlay()));	//instantly play, but let the window build first...
}

playMessage::~playMessage()
{
    CloseSound();

	delete vt;
	if(_timer) {
		delete _timer;
	}
}

void playMessage::stopClicked(bool b) {
	if(_timer->isActive()) {
		_timer->stop();
	}
	vt->stopPlaying();
	ui.progressBar->setValue(0);
	ui.progressBar->update();
	ui.labelPBarText->setText("0:00");
}

void playMessage::backClicked(bool b) {
	_timer->stop();
	vt->stopPlaying();
	emit goBack();
}

void playMessage::doPlay() {
	if(!_timer) {
		_timer = new QTimer();
		_timer->setInterval(50);
		connect(_timer,SIGNAL(timeout()),this,SLOT(updatePBar()));
	} else {
		_timer->stop();
	}

/*	if(!QSound::isAvailable()) {
		QMessageBox::warning(this,"Sound problem","QSound not available");
	} else {
		///TODO: take real file from inbox dataset
		QSound::play("voicemessage.wav");
	}*/

	ui.progressBar->setValue(0);
	ui.progressBar->update();
	_timer->start();

//	PlaySound1("voicemessage.wav",this);
//	QTimer::singleShot(1,this,SLOT(playVideo()));

	vt = new VideoThread(v);
	connect(vt,SIGNAL(updateWidget()),this,SLOT(updateVideoWidget()));
	vt->start();
}

void playMessage::updateVideoWidget() {
	v->repaint();
}

void playMessage::updatePBar() {
	if(_timer->isActive()) {
		ui.progressBar->setValue(ui.progressBar->value() + 50);
		if(ui.progressBar->value() % 1000 == 0) {
			QString s;
			int seconds = ui.progressBar->value() / 1000;
			s.sprintf("0:%s%d",(seconds > 9)?"":"0",seconds);
			ui.labelPBarText->setText(s);
		}
		///TODO: take length of sound from file
		if(ui.progressBar->value() >= 7000) {
			_timer->stop();
		}
		ui.progressBar->update();
	}
}

void playMessage::playClicked(bool b) {
	doPlay();
}
