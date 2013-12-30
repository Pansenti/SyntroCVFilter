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

#ifndef AVMUXDECODE_H
#define AVMUXDECODE_H

#include "SyntroLib.h"

class AVMuxDecode : public SyntroThread
{
    Q_OBJECT

public:
    AVMuxDecode();

public slots:
	void decoderInput(QByteArray avmuxData);

signals:
    void decoderOutput(QImage image, qint64 timestamp);

private:
    void processMJPPCM(SYNTRO_RECORD_AVMUX *avmux);
};

#endif // AVMUXDECODE_H
