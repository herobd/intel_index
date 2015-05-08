TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.cpp \
    MatchKeypoints.cpp \
    CollabKeypoints.cpp \
    wordspotlocalfeatureshmm.cpp \
    siftizedimage.cpp \
    corpus.cpp \
    codedimage.cpp \
    searcher.cpp \
    hmmquery.cpp \
    bagoffeatureshmm.cpp \
    codebook.cpp \
    simplebinnedimage.cpp \
    customsift.cpp \
    bagspotter.cpp \
    spatialaveragespotter.cpp

LIBS += -L/urs/local/include/opencv2 -l:libopencv_features2d.so.2.4 -l:libopencv_core.so.2.4 -l:libopencv_highgui.so.2.4 -lopencv_nonfree -l:libopencv_flann.so.2.4 -l:libopencv_imgproc.so.2.4
LIBS += -L/home/brian/intel_index/brian_handwriting/StochHMM/src/ -lstochhmm


QMAKE_CXXFLAGS+= -std=c++11

HEADERS += \
    CollabKeypoints.h \
    wordspotlocalfeatureshmm.h \
    FeaturizedImage.h \
    siftizedimage.h \
    corpus.h \
    codedimage.h \
    searcher.h \
    hmmquery.h \
    bagoffeatureshmm.h \
    codebook.h \
    simplebinnedimage.h \
    customsift.h \
    bagspotter.h \
    spatialaveragespotter.h
