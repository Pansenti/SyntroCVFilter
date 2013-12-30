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

#include "AVData.h"

AVData::AVData(QString streamName)
{
	m_name = streamName;
	m_decoder = NULL;
	m_filter = NULL;
	m_encoder = NULL;
	m_inputPort = -1;
	m_outputPort = -1;
	m_lastUpdate = 0;
}

AVData::~AVData()
{
	stopBackgroundProcessing();
}

QString AVData::name() const
{
	return m_name;
}

QString AVData::filteredName() const
{
	return m_name + " - Filtered";
}

qint64 AVData::lastUpdate() const
{
	return m_lastUpdate;
}

void AVData::setLastUpdate(qint64 timestamp)
{
	m_lastUpdate = timestamp;
}

QImage AVData::image()
{
	return m_image;
}

QImage AVData::filteredImage()
{
	return m_filteredImage;
}

qint64 AVData::imageTimestamp()
{
	return m_imageTimestamp;
}

int AVData::inputPort() const
{
	return m_inputPort;
}

void AVData::setInputPort(int port)
{
	m_inputPort = port;

	if (port == -1) {
		stopBackgroundProcessing();
	}
	else if (!m_decoder) {
		m_decoder = new AVMuxDecode;
		connect(this, SIGNAL(decoderInput(QByteArray)), m_decoder, SLOT(decoderInput(QByteArray)));
		connect(m_decoder, SIGNAL(decoderOutput(QImage, qint64)), this, SLOT(decoderOutput(QImage, qint64)));

		m_filter = new SimpleFilter;
		connect(this, SIGNAL(filterInput(QImage)), m_filter, SLOT(filterInput(QImage)));
		connect(m_filter, SIGNAL(filterOutput(QImage)), this, SLOT(filterOutput(QImage)));
		m_filter->resumeThread();

		m_encoderCounter = 0;
		m_encoder = new JpegEncoder;
		connect(this, SIGNAL(encoderInput(QImage)), m_encoder, SLOT(encoderInput(QImage)));
		connect(m_encoder, SIGNAL(encoderOutput(int, int, QByteArray)), this, SLOT(encoderOutput(int, int, QByteArray)));
		m_encoder->resumeThread();

		m_decoder->resumeThread();
	}
}

int AVData::outputPort() const
{
	return m_outputPort;
}

void AVData::setOutputPort(int port)
{
	m_outputPort = port;
}

void AVData::stopBackgroundProcessing()
{
	if (m_decoder) {
		disconnect(this, SIGNAL(decoderInput(QByteArray)), m_decoder, SLOT(decoderInput(QByteArray)));
		disconnect(m_decoder, SIGNAL(decoderOutput(QImage, qint64)), this, SLOT(decoderOutput(QImage, qint64)));
		m_decoder->exitThread();
		m_decoder = NULL;
	}

	if (m_filter) {
		disconnect(this, SIGNAL(filterInput(QImage)), m_filter, SLOT(filterInput(QImage)));
		disconnect(m_filter, SIGNAL(filterOutput(QImage)), this, SLOT(filterOutput(QImage)));
		m_filter->exitThread();
		m_filter = NULL;
	}

	if (m_encoder) {
		disconnect(this, SIGNAL(encoderInput(QImage)), m_encoder, SLOT(encoderInput(QImage)));
		disconnect(m_encoder, SIGNAL(encoderOutput(int, int, QByteArray)), this, SLOT(encoderOutput(int, int, QByteArray)));
		m_encoder->exitThread();
		m_encoder = NULL;
	}
}

// from the Syntro client object, feed data to the decoder
void AVData::setAVData(QByteArray rawData)
{
	emit decoderInput(rawData);
}

// signal from the decoder, processed image
void AVData::decoderOutput(QImage image, qint64 timestamp)
{
	if (!image.isNull()) {
		m_image = image;
		
		// send the image to the CVFilter
		emit filterInput(m_image);
	}

	m_imageTimestamp = timestamp;
}

// signal from the CVFilter
void AVData::filterOutput(QImage image)
{
	if (!image.isNull()) {
		m_filteredImage = image;

		// send the image to the JPEG encoder
		emit encoderInput(m_filteredImage);
	}

	m_lastUpdate = SyntroClock();
}

// signal from the JPEG encoder
void AVData::encoderOutput(int width, int height, QByteArray jpeg)
{
	// send to the Syntro client object for transmission
	if (m_outputPort >= 0)
		emit clientOutput(m_outputPort, m_encoderCounter++, width, height, jpeg);
}
