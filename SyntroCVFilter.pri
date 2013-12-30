# This file is part of Syntro
#
# Copyright (c) 2013 Pansenti, LLC. All rights reserved.
#

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += AVData.h \
	AVMuxDecode.h \
	FilterClient.h \
	ImageWindow.h \
	JpegEncoder.h \
	SimpleFilter.h \
	StreamDialog.h \
	SyntroCVFilter.h

SOURCES += AVData.cpp \
	AVMuxDecode.cpp \
	FilterClient.cpp \
	ImageWindow.cpp \
	JpegEncoder.cpp \
	main.cpp \
	SimpleFilter.cpp \
	StreamDialog.cpp \
	SyntroCVFilter.cpp

FORMS += syntrocvfilter.ui

