# This file is part of Syntro
#
# Copyright (c) 2013 Pansenti, LLC. All rights reserved.
#

cache()

TEMPLATE = app

TARGET = SyntroCVFilter

DESTDIR = Output 

QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += debug_and_release

unix {
	macx {
		LIBS += /usr/local/lib/libSyntroLib.dylib \
			/usr/local/lib/libSyntroGUI.dylib

		INCLUDEPATH += /usr/local/include/syntro \
				/usr/local/include/syntro/SyntroAV

		target.path = /usr/local/bin
	}
	else {
		CONFIG += link_pkgconfig
		PKGCONFIG += syntro

		target.path = /usr/bin
	}

	INSTALLS += target
}

DEFINES += QT_NETWORK_LIB

INCLUDEPATH += GeneratedFiles

MOC_DIR += GeneratedFiles/release

OBJECTS_DIR += release

UI_DIR += GeneratedFiles

RCC_DIR += GeneratedFiles

include(SyntroCVFilter.pri)

