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

#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include <qlabel.h>

#include "SyntroLib.h"
#include "AVData.h"

class ImageWindow : public QLabel
{
	Q_OBJECT

public:
	ImageWindow(AVData *avData, bool filtered, bool showName, bool showDate, bool showTime, QColor textColor, QWidget *parent = 0);
	virtual ~ImageWindow();

	QString name();

	void setShowName(bool enable);
	void setShowDate(bool enable);
	void setShowTime(bool enable);
	void setTextColor(QColor color);

	bool selected();
	void setSelected(bool select);

	QImage m_image;

signals:
	void imageMousePress(QString name);
	void imageDoubleClick(QString name);

protected:
	void paintEvent(QPaintEvent *event);
	void timerEvent(QTimerEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);

private:
    void newImage(QImage image, qint64 timestamp);
	QRect drawingRect();

	AVData *m_avData;
	bool m_filtered;
	QString m_name;
	bool m_showName;
	bool m_showDate;
	bool m_showTime;
	QColor m_textColor;
	bool m_selected;
	bool m_idle;
	int m_timer;
    QDate m_displayDate;
    QTime m_displayTime;
	qint64 m_lastFrame;
};

#endif // IMAGEWINDOW_H
