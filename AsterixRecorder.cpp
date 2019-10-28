#include "AsterixRecorder.h"
#include <qfiledialog.h>
#include <windows.h>
#include <qxmlstream.h>

AsterixRecorder::AsterixRecorder(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	RecvSocket = new QUdpSocket(this);
	connect(RecvSocket, SIGNAL(readyRead()), this, SLOT(RecvPacket()));

	connect(ui.BtnSelRecFile, &QAbstractButton::clicked, this, &AsterixRecorder::BtnSelRecFile_Clicked);
	connect(ui.BtnListen, &QAbstractButton::clicked, this, &AsterixRecorder::BtnListen_Clicked);
	connect(ui.BtnRecord, &QAbstractButton::clicked, this, &AsterixRecorder::BtnRecord_Clicked);
	connect(ui.BtnSelInputFile, &QAbstractButton::clicked, this, &AsterixRecorder::BtnSelInputFile_Clicked);
	connect(ui.BtnReadInputFile, &QAbstractButton::clicked, this, &AsterixRecorder::BtnReadinputFile_Clicked);
}

void AsterixRecorder::BtnSelRecFile_Clicked(void)
{
	QString filename = QFileDialog::getSaveFileName(
		this,
		"Save as",
		QDir::currentPath(),
		"ASTERIX files (*.asx);; All files (*.*)");
	if (!filename.isNull())
	{
		ui.EdRecFileName->setText(filename);
		ui.StatusTerminal->appendPlainText("Selected file to save data");
	}
	
}

void AsterixRecorder::BtnListen_Clicked(void)
{
	if (ui.BtnListen->text() == "Start listening")
	{
		ui.StatusTerminal->appendPlainText("Started listening");
		ui.BtnListen->setText("Stop listening");
		RecvSocket->bind(QHostAddress::Any, ui.EdRecvPort->text().trimmed().toInt());
	}
	else
	{
		ui.StatusTerminal->appendPlainText("Stoped listening");
		ui.BtnListen->setText("Start listening");
		RecvSocket->close();
	}
}

void AsterixRecorder::BtnRecord_Clicked(void)
{
	if (ui.EdRecFileName->text().length() == 0)
	{
		MessageBox((HWND)this->winId(), (LPCWSTR)L"Please select output file", (LPCWSTR)L"Error", MB_OK + MB_ICONEXCLAMATION);
		return;
	}

	if ((ui.BtnRecord->text() == "Start recording") && (ui.EdRecFileName->text().length() != 0))
	{
		char* RecFileName;
		RecFileName = new char[ui.EdRecFileName->text().length() + 1];

		for (int i = 0; i < ui.EdRecFileName->text().length(); i++)
		{
			RecFileName[i] = ui.EdRecFileName->text().data()[i].toLatin1();
			RecFileName[ui.EdRecFileName->text().length()] = 0x00;
		}

		ui.StatusTerminal->appendPlainText("Started recording");
		ui.BtnRecord->setText("Stop recording");
		OutFile.open(RecFileName, std::ofstream::out | std::ofstream::binary);		
	}
	else
	{
		ui.StatusTerminal->appendPlainText("Stoped recording");
		ui.BtnRecord->setText("Start recording");
		OutFile.flush();
		OutFile.close();
	}
}

void AsterixRecorder::BtnSelInputFile_Clicked(void)
{
	QString filename = QFileDialog::getOpenFileName(
		this,
		"Open",
		QDir::currentPath(),
		"All supported files (*.asx *.xml);; ASTERIX files (*.asx);; Wireshark PDML XML (*.xml)");
	if (!filename.isNull())
	{
		ui.EdInputFileName->setText(filename);
		ui.StatusTerminal->appendPlainText("Selected input file \n");
	}
}

void AsterixRecorder::BtnReadinputFile_Clicked(void)
{
	QXmlStreamReader::TokenType token;
	QXmlStreamAttributes attributes;

	QFile inputfile(ui.EdInputFileName->text());
	

	if (inputfile.open(QIODevice::ReadOnly))
	{
		uint8_t isFirst=1;
		long long int firstTimestamp=0;
		long long int curTimestamp = 0;
		char diffTimestamp[17];
		QString rawTimestamp;

		QXmlStreamReader inputXml(&inputfile);
		while (!inputXml.atEnd() && !inputXml.hasError())
		{
			token = inputXml.readNext();
			if (inputXml.name() == "field")
			{
				attributes = inputXml.attributes();
				if (attributes.value("name").toString() == "timestamp")
				{
					if (isFirst)
					{
						isFirst = 0;
						rawTimestamp = attributes.value("value").toString().replace('.', ',');
						firstTimestamp = atoi(rawTimestamp.toLocal8Bit());
						ui.StatusTerminal->insertPlainText("Offset: 0 ");
					}
					else
					{
						rawTimestamp = attributes.value("value").toString().replace('.', ',');
						curTimestamp = atoi(rawTimestamp.toLocal8Bit());
						itoa(curTimestamp - firstTimestamp, diffTimestamp, 10);
						ui.StatusTerminal->insertPlainText("Offset: ");
						ui.StatusTerminal->insertPlainText(diffTimestamp);
						ui.StatusTerminal->insertPlainText(" ");
					}
				}
			}

			if (inputXml.name() == "field")
			{
				attributes = inputXml.attributes();
				if (attributes.value("name").toString() == "data")
				{
					ui.StatusTerminal->insertPlainText("Data: " + attributes.value("value").toString() + "\n");
				}
			}
		}
		inputfile.close();
	}


}

void AsterixRecorder::RecvPacket(void)
{
	char * RecvData;
	uint16_t RecvDataSize;

	while (RecvSocket->hasPendingDatagrams())
	{
		RecvDataSize = RecvSocket->pendingDatagramSize();
		RecvData = new char[RecvDataSize];

		RecvSocket->readDatagram(RecvData, RecvDataSize);
		ui.StatusTerminal->appendPlainText("UDP packet received. Packet size: " + QString::number(RecvDataSize) + " bytes.");
		if (OutFile.is_open())
		{
			if ((RecvData[0] == 1) || (RecvData[0] == 2) || (RecvData[0] == -6))
			{
				OutFile.write(RecvData, RecvDataSize);
			}
		}

		delete[] RecvData;
	}
	
}

//MessageBox((HWND)this->winId(), (LPCWSTR)L"Button ckicked!", (LPCWSTR)L"Notification", MB_OK + MB_ICONINFORMATION);