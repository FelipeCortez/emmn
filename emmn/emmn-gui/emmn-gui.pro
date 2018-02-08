#-------------------------------------------------
#
# Project created by QtCreator 2017-04-11T14:42:35
#
#-------------------------------------------------

QT       += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = emmn-gui
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
           mainwindow.cpp \
           tracker.cpp \
           addtrackerdialog.cpp \
           control.cpp \
           serial.cpp \
           trackerlistmodel.cpp \
           network.cpp \
           settingsdialog.cpp \
           nextpassesview.cpp \
           helpers.cpp \
           manualcontroldialog.cpp \
           joystickwidget.cpp \
           logger.cpp

HEADERS += mainwindow.h \
           tracker.h \
           addtrackerdialog.h \
           control.h \
           serial.h \
           trackerlistmodel.h \
           network.h \
           settingsdialog.h \
           nextpassesview.h \
           helpers.h \
           manualcontroldialog.h \
           joystickwidget.h \
           logger.h

FORMS   += mainwindow.ui \
           settingsdialog.ui \
           manualcontroldialog.ui \
           trackerdialog.ui

LIBS += $$PWD\..\sgp4\build\libsgp4\libsgp4.a

INCLUDEPATH += $$PWD\..\sgp4\build\libsgp4\include

DISTFILES += credentials.txt

RESOURCES += resources.qrc
