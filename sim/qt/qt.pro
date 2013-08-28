#-------------------------------------------------
#
# Project created by QtCreator 2013-03-25T14:53:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ../../qtna
TEMPLATE = app

SOURCES += \
        ../entity/*.c\
        ../graph/*.c\
        ../gui/*.c\
        ../universe/*.c\
        ../noble/*.c\
        mainwindow.cpp \
        main.cpp


HEADERS  += mainwindow.h \
    ../noble/noble.h \
    ../gui/gui.h \
    ../gui/shared.h \
    ../entity/entity_internal.h \
    ../entity/entity.h \
    ../universe/universe.h \
    ../graph/phosphene.h \
    ../graph/graph.h

FORMS    += mainwindow.ui
