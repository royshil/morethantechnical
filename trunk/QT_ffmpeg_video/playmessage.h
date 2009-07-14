#ifndef PLAYMESSAGE_H
#define PLAYMESSAGE_H

#include <QtGui/QWidget>
#include "ui_playmessage.h"
#include "VideoWidget.h"
#include "/usr/include/qt4/QtCore/qmutex.h"
#include "VideoThread.h"

class playMessage : public QWidget
{
    Q_OBJECT

public:
    playMessage(QWidget *parent = 0);
    ~playMessage();
    void play() {doPlay();};

signals:
	void goBack();

private:
    Ui::playMessageClass ui;
    QTimer* _timer;
    VideoWidget* v;
    VideoThread* vt;

private slots:
	void doPlay();
	void backClicked(bool);
    void updatePBar();
    void playClicked(bool);
    void updateVideoWidget();
    void stopClicked(bool);
};

#endif // PLAYMESSAGE_H
