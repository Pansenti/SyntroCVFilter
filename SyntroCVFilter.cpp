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

#include <qcolordialog.h>
#include <qgridlayout.h>

#include "SyntroCVFilter.h"
#include "SyntroAboutDlg.h"
#include "BasicSetupDlg.h"
#include "StreamDialog.h"


SyntroCVFilter::SyntroCVFilter(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	m_logTag = PRODUCT_TYPE;

	SyntroUtils::syntroAppInit();
	m_client = new FilterClient;

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	
	connect(m_client, SIGNAL(clientConnected()), this, SLOT(clientConnected()));
	connect(m_client, SIGNAL(clientClosed()), this, SLOT(clientClosed()));

	connect(m_client, SIGNAL(dirResponse(QStringList)), this, SLOT(dirResponse(QStringList)));
	connect(this, SIGNAL(requestDir()), m_client, SLOT(requestDir()));

	connect(this, SIGNAL(enableService(AVData *)), m_client, SLOT(enableService(AVData *)));
	connect(this, SIGNAL(disableService(int, int)), m_client, SLOT(disableService(int, int)));

	m_client->resumeThread();

	m_statusTimer = startTimer(2000);
	m_directoryTimer = startTimer(10000);

	restoreWindowState();
	initStatusBar();
	initMenus();
	layoutGrid();

	setWindowTitle(QString("%1 - %2").arg(SyntroUtils::getAppType()).arg(SyntroUtils::getAppName()));
}

void SyntroCVFilter::closeEvent(QCloseEvent *)
{
	killTimer(m_statusTimer);
	killTimer(m_directoryTimer);

	if (m_client) {
		m_client->exitThread();
		m_client = NULL;
	}

	saveWindowState();

	SyntroUtils::syntroAppExit();
}

void SyntroCVFilter::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_directoryTimer) {
		emit requestDir();

		while (m_delayedDeleteList.count() > 0) {
			qint64 lastUpdate = m_delayedDeleteList.at(0)->lastUpdate();

			if (!SyntroUtils::syntroTimerExpired(SyntroClock(), lastUpdate, 5 * SYNTRO_CLOCKS_PER_SEC))
				break;
				
			AVData *avData = m_delayedDeleteList.at(0);
			m_delayedDeleteList.removeAt(0);	
			delete avData;
		}
	}
	else {
		m_controlStatus->setText(m_client->getLinkState());
	}
}

void SyntroCVFilter::clientConnected()
{
	emit requestDir();
}

void SyntroCVFilter::clientClosed()
{
	ui.actionVideoStreams->setEnabled(false);
	m_clientDirectory.clear();
}

void SyntroCVFilter::dirResponse(QStringList directory)
{
	m_clientDirectory = directory;

	if (m_clientDirectory.length() > 0) {
		if (!ui.actionVideoStreams->isEnabled())
			ui.actionVideoStreams->setEnabled(true);
	}
}

void SyntroCVFilter::onChooseVideoStreams()
{
	QStringList oldStreams;

	for (int i = 0; i < m_avData.count(); i++)
		oldStreams.append(m_avData.at(i)->name());

	StreamDialog dlg(this, m_clientDirectory, oldStreams);

	if (dlg.exec() != QDialog::Accepted)
		return;

	QStringList newStreams = dlg.newStreams();

	QList<AVData *> oldSourceList = m_avData;
	m_avData.clear();

	// don't tear down existing streams if we can rearrange them
	for (int i = 0; i < newStreams.count(); i++) {
		int oldIndex = oldStreams.indexOf(newStreams.at(i));

		if (oldIndex == -1)
			addAVSource(newStreams.at(i));
		else
			m_avData.append(oldSourceList.at(oldIndex));
	}

	// delete streams we no longer need
	for (int i = 0; i < oldStreams.count(); i++) {
		if (newStreams.contains(oldStreams.at(i)))
			continue;

		AVData *avData = oldSourceList.at(i);

		if (avData->inputPort() >= 0) {
			emit disableService(avData->inputPort(), avData->outputPort());

			avData->setInputPort(-1);
			avData->setOutputPort(-1);
			avData->stopBackgroundProcessing();
		}
		
		// we will delete 5 seconds from now
		avData->setLastUpdate(SyntroClock());
		m_delayedDeleteList.append(avData);
	}

	layoutGrid();
}

bool SyntroCVFilter::addAVSource(QString name)
{
	AVData *avData = new AVData(name);

	if (!avData)
		return false;

	m_avData.append(avData);

	emit enableService(avData);

	return true;
}

void SyntroCVFilter::layoutGrid()
{
	int count = m_avData.count();

	if (count > 4)
		count = 4;

	m_windowList.clear();

	QWidget *newCentralWidget = new QWidget();

	QGridLayout *grid = new QGridLayout(this);
	grid->setSpacing(3);
	grid->setContentsMargins(1, 1, 1, 1);
	
	for (int i = 0; i < count; i++) {
		ImageWindow *win = new ImageWindow(m_avData.at(i), false, m_showName, m_showDate, m_showTime, m_textColor, newCentralWidget);
		m_windowList.append(win);
		grid->addWidget(win, i, 0);

		win = new ImageWindow(m_avData.at(i), true, m_showName, m_showDate, m_showTime, m_textColor, newCentralWidget);
		m_windowList.append(win);
		grid->addWidget(win, i, 1);
	}

	for (int i = 0; i < count; i++)
		grid->setRowStretch(i, 1);

	grid->setColumnStretch(0, 1);
	grid->setColumnStretch(1, 1);

	newCentralWidget->setLayout(grid);

	QWidget *oldCentralWidget = centralWidget();

	setCentralWidget(newCentralWidget);

	delete oldCentralWidget;
}

void SyntroCVFilter::initStatusBar()
{
	m_controlStatus = new QLabel(this);
	m_controlStatus->setAlignment(Qt::AlignLeft);
	ui.statusBar->addWidget(m_controlStatus, 1);
}

void SyntroCVFilter::initMenus()
{
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
	connect(ui.actionBasicSetup, SIGNAL(triggered()), this, SLOT(onBasicSetup()));

	connect(ui.actionVideoStreams, SIGNAL(triggered()), this, SLOT(onChooseVideoStreams()));
	ui.actionVideoStreams->setEnabled(false);

	connect(ui.actionShowName, SIGNAL(triggered()), this, SLOT(onShowName()));
	connect(ui.actionShowDate, SIGNAL(triggered()), this, SLOT(onShowDate()));
	connect(ui.actionShowTime, SIGNAL(triggered()), this, SLOT(onShowTime()));
	connect(ui.actionTextColor, SIGNAL(triggered()), this, SLOT(onTextColor()));

	ui.actionShowName->setChecked(m_showName);
	ui.actionShowDate->setChecked(m_showDate);
	ui.actionShowTime->setChecked(m_showTime);
}

void SyntroCVFilter::onShowName()
{
	m_showName = ui.actionShowName->isChecked();

	for (int i = 0; i < m_windowList.count(); i++)
		m_windowList[i]->setShowName(m_showName);
}

void SyntroCVFilter::onShowDate()
{
	m_showDate = ui.actionShowDate->isChecked();

	for (int i = 0; i < m_windowList.count(); i++)
		m_windowList[i]->setShowDate(m_showDate);
}

void SyntroCVFilter::onShowTime()
{
	m_showTime = ui.actionShowTime->isChecked();

	for (int i = 0; i < m_windowList.count(); i++)
		m_windowList[i]->setShowTime(m_showTime);
}

void SyntroCVFilter::onTextColor()
{
	m_textColor = QColorDialog::getColor(m_textColor, this);

	for (int i = 0; i < m_windowList.count(); i++)
		m_windowList[i]->setTextColor(m_textColor);
}

void SyntroCVFilter::onAbout()
{
	SyntroAbout dlg(this);
	dlg.exec();
}

void SyntroCVFilter::onBasicSetup()
{
	BasicSetupDlg dlg(this);
	dlg.exec();
}

void SyntroCVFilter::saveWindowState()
{
	QSettings *settings = SyntroUtils::getSettings();

	settings->beginGroup("Window");
	settings->setValue("Geometry", saveGeometry());
	settings->setValue("State", saveState());
	settings->setValue("showName", m_showName);
	settings->setValue("showDate", m_showDate);
	settings->setValue("showTime", m_showTime);
	settings->setValue("textColor", m_textColor);
	settings->endGroup();
	
	settings->beginWriteArray("streamSources");

	for (int i = 0; i < m_avData.count(); i++) {
		settings->setArrayIndex(i);
		settings->setValue("source", m_avData[i]->name());
	}

	settings->endArray();

	delete settings;
}

void SyntroCVFilter::restoreWindowState()
{
	QSettings *settings = SyntroUtils::getSettings();

	settings->beginGroup("Window");
	restoreGeometry(settings->value("Geometry").toByteArray());
	restoreState(settings->value("State").toByteArray());

	if (settings->contains("showName")) 
		m_showName = settings->value("showName").toBool();
	else
		m_showName = true;

	if (settings->contains("showDate"))
		m_showDate = settings->value("showDate").toBool();
	else
		m_showDate = true;

	if (settings->contains("showTime"))
		m_showTime = settings->value("showTime").toBool();
	else
		m_showTime = true;

	if (settings->contains("textColor"))
		m_textColor = settings->value("textColor").value<QColor>();
	else
		m_textColor = Qt::white;

	settings->endGroup();
	
	int count = settings->beginReadArray("streamSources");
	
	for (int i = 0; i < count; i++) {
		settings->setArrayIndex(i);
		QString name = settings->value("source", "").toString();

		if (name.length() > 0)
			addAVSource(name);
	}
	
	settings->endArray();

	delete settings;
}
