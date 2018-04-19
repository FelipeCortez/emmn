QT       += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = emmn-gui
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
          main.cpp \
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
           logger.cpp \
           soltrack.cpp
HEADERS += \
    mainwindow.h \
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
   logger.h \
   soltrack.h
FORMS += \
    mainwindow.ui \
    settingsdialog.ui \
    manualcontroldialog.ui \
    trackerdialog.ui

include(../emmn.pri)
