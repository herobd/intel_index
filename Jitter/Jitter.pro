TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

LIBS += -L/urs/local/include/opencv2 -lopencv_features2d -lopencv_core -lopencv_highgui
