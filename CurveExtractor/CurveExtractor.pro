#-------------------------------------------------
#
# Project created by QtCreator 2014-08-06T10:41:04
#
#-------------------------------------------------

#QT       += core

#QT       -= gui

TARGET = CurveExtractor
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += qt

TEMPLATE = app



SOURCES += main.cpp \
    ../nameseparation/bpartition.cpp \
    ../nameseparation/bimage.cpp

HEADERS += \
    ../nameseparation/BPixelCollection.h \
    ../nameseparation/bpartition.h \
    ../nameseparation/bimage.h

LIBS += -lgsl
LIBS += -lgslcblas
LIBS += -lm
