#-------------------------------------------------
#
# Project created by QtCreator 2012-10-02T23:29:28
#
#-------------------------------------------------

QT       += core

QT       += gui

TARGET = UnityImageOut
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
SOURCES += main.cpp
QT += widgets
HEADERS += \
    pvrtc_dll.h

LIBS += -L../UnityImageOut -lpvrtc


