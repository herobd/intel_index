TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += debug_and_release

QMAKE_CXX = g++-4.9
QMAKE_CXXFLAGS+= -std=c++11

SOURCES += main.cpp

#includeing Doug's code
INCLUDEPATH +=/home/brian/familysearch_documentproject_2013.08.30/src
LIBS += -L/home/brian/familysearch_documentproject_2013.08.30/lib/ -ldocumentproj_2013.08.30 -ljpeg -ltiff -lpng -lpthread
