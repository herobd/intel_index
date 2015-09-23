#-------------------------------------------------
#
# Project created by QtCreator 2015-09-18T17:17:45
#
#-------------------------------------------------

QT       -= core gui

TARGET = EmbAttSpotter
TEMPLATE = lib

DEFINES += EMBATTSPOTTER_LIBRARY

SOURCES += embattspotter.cpp \
    evaluator.cpp

HEADERS += embattspotter.h\
        embattspotter_global.h \
    evaluator.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
