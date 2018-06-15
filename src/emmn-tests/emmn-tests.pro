QT += testlib core serialport

CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES += \
    tst_helperstest.cpp \
    $$PWD\..\emmn-gui\helpers.cpp \
    $$PWD\..\emmn-gui\tracker.cpp \
    $$PWD\..\emmn-gui\trackerlistmodel.cpp


INCLUDEPATH += $$PWD\..\emmn-gui

include(../emmn.pri)
