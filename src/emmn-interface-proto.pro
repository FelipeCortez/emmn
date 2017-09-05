#-------------------------------------------------
#
# Project created by QtCreator 2017-04-11T14:42:35
#
#-------------------------------------------------

QT       += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = emmn-interface-proto
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
        tracker.cpp \
        addtrackerdialog.cpp \
        control.cpp \
        efem.cpp \
        serial.cpp \
        trackerlistmodel.cpp \
        network.cpp \
        settingsdialog.cpp \
        nextpassesview.cpp \
        helpers.cpp \
        manualcontroldialog.cpp \
        joystickwidget.cpp

HEADERS += mainwindow.h \
        tracker.h \
        addtrackerdialog.h \
        control.h \
        efem.h \
        serial.h \
        trackerlistmodel.h \
        network.h \
        settingsdialog.h \
        nextpassesview.h \
        helpers.h \
        manualcontroldialog.h \
        joystickwidget.h

FORMS   += mainwindow.ui \
        tracker_dialog.ui \
        settings_dialog.ui \
        manualcontrol_dialog.ui

LIBS +=        ..\sgp4\build\libsgp4\libsgp4.a
INCLUDEPATH += ..\sgp4\build\libsgp4\include
#LIBS +=        C:\Users\Inpe\Documents\FelipeC\sgp4\build\libsgp4\libsgp4.a
#INCLUDEPATH += C:\Users\Inpe\Documents\FelipeC\sgp4\build\libsgp4\include