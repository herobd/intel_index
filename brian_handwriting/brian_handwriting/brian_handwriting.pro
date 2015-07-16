TEMPLATE = app
CONFIG += console
CONFIG -= qt

QMAKE_CXX = g++-4.9

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
    spatialaveragespotter.cpp \
    gatosbinarize.cpp \
    enhancedbovw.cpp \
    hog.cpp \
    kmeans_tbb.cpp \
    liang.cpp \
    mog.cpp \
    grapheme.cpp

LIBS += -L/urs/local/include/opencv2 -l:libopencv_features2d.so.2.4 -l:libopencv_core.so.2.4 -l:libopencv_highgui.so.2.4 -lopencv_nonfree -l:libopencv_flann.so.2.4 -l:libopencv_imgproc.so.2.4 -l:libopencv_objdetect.so.2.4
#LIBS += -L/home/brian/intel_index/brian_handwriting/StochHMM/src/ -lstochhmm
LIBS += -fopenmp

QMAKE_CXXFLAGS+= -std=c++11 -fopenmp

#includeing Doug's code
INCLUDEPATH +=/home/brian/familysearch_documentproject_2013.08.30/src
LIBS += -L/home/brian/familysearch_documentproject_2013.08.30/lib/ -ldocumentproj_2013.08.30 -ljpeg -ltiff -lpng


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
    spatialaveragespotter.h \
    MatchKeypoints.h \
    gatosbinarize.h \
    enhancedbovw.h \
    hog.h \
    liang.h \
    mog.h \
    grapheme.h


QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE *= -O2
