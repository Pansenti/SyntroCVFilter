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

#ifndef AVDATA_H
#define AVDATA_H

#include "AVMuxDecode.h"
#include "JpegEncoder.h"
#include "SimpleFilter.h"

class AVData : public QObject
{
	Q_OBJECT

public:
	AVData(QString streamName);
	~AVData();

	QString name() const;
	QString filteredName() const;

	int inputPort() const;
	void setInputPort(int port);

	int outputPort() const;
	void setOutputPort(int port);

	qint64 lastUpdate() const;
	void setLastUpdate(qint64 timestamp);

	QImage image();
	QImage filteredImage();
	qint64 imageTimestamp();
	
	void setAVData(QByteArray rawData);
	void stopBackgroundProcessing();

signals:
	void decoderInput(QByteArray avmuxData);
	void filterInput(QImage image);
	void encoderInput(QImage image);
	void clientOutput(int outputPort, int counter, int width, int height, QByteArray jpeg);

public slots:	
	void decoderOutput(QImage image, qint64 timestamp);
	void filterOutput(QImage image);
	void encoderOutput(int width, int height, QByteArray jpeg);

private:
	QString m_name;
	int m_inputPort;
	int m_outputPort;

	qint64 m_lastUpdate;
	QImage m_image;
	QImage m_filteredImage;
	qint64 m_imageTimestamp;

	AVMuxDecode *m_decoder;
	SimpleFilter *m_filter;
	JpegEncoder *m_encoder;

	int m_encoderCounter;
};

#endif // AVDATA_H
