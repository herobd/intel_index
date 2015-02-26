TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.cpp \
    MatchKeypoints.cpp

LIBS += -L/urs/local/include/opencv2 -l:libopencv_features2d.so.2.4 -l:libopencv_core.so.2.4 -l:libopencv_highgui.so.2.4 -lopencv_nonfree -l:libopencv_flann.so.2.4
QMAKE_CXXFLAGS+= -std=c++11
