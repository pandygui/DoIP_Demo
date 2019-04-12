#-------------------------------------------------
#
# Project created by QtCreator 2019-03-23T10:04:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Vehicle
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mytest.cpp \
    DoIP_pc.cpp

HEADERS  += mainwindow.h \
    mytest.h \
    DoIP_pc.h

FORMS    += mainwindow.ui

RESOURCES += \
    image.qrc
