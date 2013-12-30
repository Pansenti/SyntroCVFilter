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

#ifndef FILTERCLIENT_H
#define FILTERCLIENT_H

#include "SyntroLib.h"
#include "AVData.h"

class FilterClient : public Endpoint
{
	Q_OBJECT

public:
    FilterClient();

public slots:
	void enableService(AVData *avData);
	void disableService(int inputPort, int outputPort);
	void requestDir();
	void newOutput(int outputPort, int counter, int width, int height, QByteArray jpeg);

signals:
	void clientConnected();
	void clientClosed();
	void dirResponse(QStringList directory);

protected:
	void appClientConnected();
	void appClientClosed();
    void appClientReceiveMulticast(int servicePort, SYNTRO_EHEAD *multiCast, int len);
    void appClientReceiveDirectory(QStringList);

private:
	SYNTRO_AVPARAMS m_avParams;
};

#endif // FILTERCLIENT_H
