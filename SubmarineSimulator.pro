#-------------------------------------------------
#
# Project created by QtCreator 2016-02-29T09:52:52
#
#-------------------------------------------------

QT += core gui widgets
QT += 3dcore 3drenderer 3dinput

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig
PKGCONFIG += bullet

CONFIG += c++11

TARGET = SubmarineSimulator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    simulation.cpp \
    submarine.cpp \
    fluid.cpp \
    forcearrow.cpp

HEADERS  += mainwindow.h \
    simulation.h \
    submarine.h \
    fluid.h \
    forcearrow.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
