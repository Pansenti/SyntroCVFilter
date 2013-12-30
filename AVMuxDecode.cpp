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

#include "AVMuxDecode.h"

#include <qimage.h>
#include <qbuffer.h>
#include <qbytearray.h>

AVMuxDecode::AVMuxDecode() 
	: SyntroThread("AVMuxDecode", "AVMuxDecode")
{
}

void AVMuxDecode::decoderInput(QByteArray avmuxData)
{
	SYNTRO_AVPARAMS params;

	if (avmuxData.size() < sizeof(SYNTRO_RECORD_AVMUX))
		return;

    SYNTRO_RECORD_AVMUX *avmux = (SYNTRO_RECORD_AVMUX *)avmuxData.constData();

	if (SYNTRO_RECORDHEADER_PARAM_NOOP == SyntroUtils::convertUC2ToInt(avmux->recordHeader.param))
		return;

    SyntroUtils::avmuxHeaderToAVParams(avmux, &params);
    
	if (SYNTRO_RECORD_TYPE_AVMUX_MJPPCM == params.avmuxSubtype)
		processMJPPCM(avmux);
	else
		logDebug(QString("Unsupported avmux type %1").arg(params.avmuxSubtype));
}

void AVMuxDecode::processMJPPCM(SYNTRO_RECORD_AVMUX *avmux)
{
	int videoSize = SyntroUtils::convertUC4ToInt(avmux->videoSize);

	// could be audio data with no video, can ignore    
	if (videoSize == 0)
		return;

	// this is an error
	if ((videoSize < 0) || (videoSize > 300000)) {
		logDebug(QString("Illegal video data size %1").arg(videoSize));
		return;
	}

	unsigned char *ptr = (unsigned char *)(avmux + 1) + SyntroUtils::convertUC4ToInt(avmux->muxSize);

	QImage image;
        
	if (image.loadFromData((const uchar *)ptr, videoSize, "JPEG"))
		emit decoderOutput(image, SyntroUtils::convertUC8ToInt64(avmux->recordHeader.timestamp));
}
