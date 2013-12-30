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

#ifndef SYNTROCVFILTER_H
#define SYNTROCVFILTER_H

#include <qmainwindow.h>
#include "ui_syntrocvfilter.h"

#include "FilterClient.h"
#include "ImageWindow.h"

#define PRODUCT_TYPE "SyntroCVFilter"

class SyntroCVFilter : public QMainWindow
{
	Q_OBJECT

public:
	SyntroCVFilter(QWidget *parent = 0);

public slots:
	void onAbout();
	void onBasicSetup();
	void onChooseVideoStreams();
    void onShowName();
	void onShowDate();
	void onShowTime();
	void onTextColor();
	void clientConnected();
	void clientClosed();
	void dirResponse(QStringList directory);

signals:
	void requestDir();
	void enableService(AVData *avData);
	void disableService(int sourcePort, int destPort);

protected:
	void closeEvent(QCloseEvent *event);
	void timerEvent(QTimerEvent *event);

private:
	bool addAVSource(QString name);
	void layoutGrid();
	void initStatusBar();
	void initMenus();
	void saveWindowState();
	void restoreWindowState();

	Ui::SyntroCVFilterClass ui;

	QString m_logTag;
	FilterClient *m_client;
	QStringList m_clientDirectory;

	QList<AVData *> m_avData;
	QList<ImageWindow *> m_windowList;
	QList<AVData *> m_delayedDeleteList;

	QLabel *m_controlStatus;

	int m_statusTimer;
	int m_directoryTimer;

	bool m_showName;
	bool m_showDate;
	bool m_showTime;
	QColor m_textColor;
};

#endif // SYNTROCVFILTER_H
