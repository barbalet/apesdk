#-------------------------------------------------
#
# Project created by QtCreator 2013-03-25T14:53:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ../../qtna
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        ../gui/*.c\
        ../universe/*.c\
        ../noble/*.c\
        ../entity/*.c

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
