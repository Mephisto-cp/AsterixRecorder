#pragma once

#include <QtWidgets/QMainWindow>
#include <QUdpSocket>
#include "ui_AsterixRecorder.h"
#include <fstream>

class AsterixRecorder : public QMainWindow
{
	Q_OBJECT

public:
	AsterixRecorder(QWidget *parent = Q_NULLPTR);

private:
	Ui::AsterixRecorderClass ui;
	QUdpSocket *RecvSocket;
	std::ofstream OutFile;

public slots:
	//Record tab elements
	void BtnSelRecFile_Clicked(void);
	void BtnListen_Clicked(void);
	void BtnRecord_Clicked(void);
	//Playback tab elements
	void BtnSelInputFile_Clicked(void);
	void BtnReadinputFile_Clicked(void);
	//Other functions
	void RecvPacket(void);
};
