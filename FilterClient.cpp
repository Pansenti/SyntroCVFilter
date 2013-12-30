//
//  Copyright (c) 2012, 2013 Pansenti, LLC.
//	
//  This file is part of Syntro
//
//  Syntro is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Syntro is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Syntro.  If not, see <http://www.gnu.org/licenses/>.
//

#include "FilterClient.h"

#define	FILTERCLIENT_BACKGROUND_INTERVAL (SYNTRO_CLOCKS_PER_SEC/100)


FilterClient::FilterClient()
    : Endpoint(FILTERCLIENT_BACKGROUND_INTERVAL, "FilterClient")
{
	// save some time and do this once, setup for mjpeg, no audio
	memset(&m_avParams, 0, sizeof(m_avParams));

	m_avParams.avmuxSubtype = SYNTRO_RECORD_TYPE_AVMUX_MJPPCM;
	m_avParams.videoSubtype = SYNTRO_RECORD_TYPE_VIDEO_MJPEG;
	m_avParams.audioSubtype = SYNTRO_RECORD_TYPE_AUDIO_UNKNOWN;
}

void FilterClient::appClientConnected()
{
	emit clientConnected();
}

void FilterClient::appClientClosed()
{
	emit clientClosed();
}

void FilterClient::requestDir()
{
	requestDirectory();
}

void FilterClient::appClientReceiveDirectory(QStringList directory)
{
	emit dirResponse(directory);
}

void FilterClient::enableService(AVData *avData)
{
	QString inputService = SyntroUtils::insertStreamNameInPath(avData->name(), SYNTRO_STREAMNAME_AVMUX);

	int inputPort = clientAddService(inputService, SERVICETYPE_MULTICAST, false);

	if (inputPort < 0)
		return;

	QString outputService = QString("%1:%2").arg(SYNTRO_STREAMNAME_AVMUX).arg(avData->name());
	
	int outputPort = clientAddService(outputService, SERVICETYPE_MULTICAST, true);

	if (outputPort < 0) {
		clientRemoveService(inputPort);
		return;
	}

	clientSetServiceDataPointer(inputPort, (void *)avData);
	avData->setInputPort(inputPort);
	avData->setOutputPort(outputPort);
	connect(avData, SIGNAL(clientOutput(int, int, int, int, QByteArray)),
		this, SLOT(newOutput(int, int, int, int, QByteArray)));
}

void FilterClient::disableService(int inputPort, int outputPort)
{
	AVData *avData = reinterpret_cast<AVData *>(clientGetServiceDataPointer(inputPort));

	if (avData) {
		disconnect(avData, SIGNAL(clientOutput(int, int, int, int, QByteArray)),
			this, SLOT(newOutput(int, int, int, int, QByteArray)));
	}

	clientRemoveService(inputPort);
	clientRemoveService(outputPort);
}

void FilterClient::appClientReceiveMulticast(int servicePort, SYNTRO_EHEAD *multiCast, int len)
{
	AVData *avData = reinterpret_cast<AVData *>(clientGetServiceDataPointer(servicePort));

	if (avData) {
		SYNTRO_RECORD_AVMUX *avmuxHeader = reinterpret_cast<SYNTRO_RECORD_AVMUX *>(multiCast + 1);
		int recordType = SyntroUtils::convertUC2ToUInt(avmuxHeader->recordHeader.type);

		if (recordType != SYNTRO_RECORD_TYPE_AVMUX) {
			qDebug() << "Expecting avmux record, received record type" << recordType;
		}
		else {
			avData->setAVData(QByteArray((const char *)avmuxHeader, len));
			clientSendMulticastAck(servicePort);
		}
	}
	else {
		logWarn(QString("Multicast received to invalid port %1").arg(servicePort));
	}

	free(multiCast);
}

void FilterClient::newOutput(int outputPort, int counter, int width, int height, QByteArray jpeg)
{
	if (!clientIsServiceActive(outputPort))
		return;

	if (!clientClearToSend(outputPort))
		return;

	m_avParams.videoWidth = width;
	m_avParams.videoHeight = height;

	// total packet length
	int length = sizeof(SYNTRO_RECORD_AVMUX) + (int)jpeg.size();

	// Request a Syntro a mulicast packet with extra room for our data
	// Our data consists of a video header and the actual image
	// Syntro will fill in the SYNTRO_EHEAD multicast header
	SYNTRO_EHEAD *multiCast = clientBuildMessage(outputPort, length);

	// the avmux data starts after the multicast header
	SYNTRO_RECORD_AVMUX *avmuxHead = (SYNTRO_RECORD_AVMUX *)(multiCast + 1);
	
	// A utility function fills in the avmux header for us
	SyntroUtils::avmuxHeaderInit(avmuxHead, &m_avParams, SYNTRO_RECORDHEADER_PARAM_NORMAL,
		counter, 0, (int)jpeg.size(), 0);

	// Since we only have image data, append it right after the avmux header
	memcpy((unsigned char *)(avmuxHead + 1), jpeg.constData(), jpeg.size());

	// send it
	clientSendMessage(outputPort, multiCast, length, SYNTROLINK_MEDPRI);
}