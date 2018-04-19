QT += testlib
QT += gui network serialport

CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES +=  tst_testfirst.cpp

include(../emmn.pri)
