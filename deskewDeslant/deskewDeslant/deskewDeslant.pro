#-------------------------------------------------
#
# Project created by QtCreator 2015-06-25T14:39:35
#
#-------------------------------------------------

QT       += testlib

#QT       -= gui

TARGET = tst_deskewdeslanttest
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += qt

TEMPLATE = app


SOURCES += \
    main.cpp \
    dedgedetector.cpp \
    dglobalskew.cpp \
    dprofile.cpp \
    dthresholder.cpp \
    dconvolver.cpp \
    dimage.cpp \
    dkernel2d.cpp \
    dprogress.cpp \
    dsize.cpp \
    dinstancecounter.cpp \
    dconnectedcomplabeler.cpp \
    dtimer.cpp \
    dimageio.cpp \
    drect.cpp \
    dpoint.cpp \
    dconnectedcompinfo.cpp \
    nameseparation/wordseparator.cpp \
    nameseparation/wordprofile.cpp \
    nameseparation/graphcut.cpp \
    nameseparation/distancetransform.cpp \
    nameseparation/dimension.cpp \
    nameseparation/bpartition.cpp \
    nameseparation/boxcleaner.cpp \
    nameseparation/bimage.cpp \
    nameseparation/maxflow/maxflow.cpp \
    nameseparation/maxflow/graph.cpp \
    nameseparation/angleimage.cpp \
    nameseparation/blobskeleton.cpp \
    nameseparation/pathstackmap.cpp \
    nameseparation/operators.cpp \
    dtextlineseparator.cpp \
    dslantangle.cpp \
    dtcm.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    dedgedetector.h \
    dglobalskew.h \
    dmath.h \
    dprofile.h \
    dthresholder.h \
    dconvolver.h \
    dimage.h \
    dkernel2d.h \
    ddefs.h \
    dprogress.h \
    dinttypes.h \
    dsize.h \
    dinstancecounter.h \
    dconnectedcomplabeler.h \
    dtimer.h \
    dthreads.h \
    dmemalign.h \
    dimageio.h \
    drect.h \
    dpoint.h \
    dconnectedcompinfo.h \
    nameseparation/wordseparator.h \
    nameseparation/wordprofile.h \
    nameseparation/graphcut.h \
    nameseparation/distancetransform.h \
    nameseparation/dimension.h \
    nameseparation/Constants.h \
    nameseparation/BPixelCollection.h \
    nameseparation/bpartition.h \
    nameseparation/boxcleaner.h \
    nameseparation/bimage.h \
    nameseparation/maxflow/graph.h \
    nameseparation/maxflow/block.h \
    nameseparation/angleimage.h \
    nameseparation/blobskeleton.h \
    nameseparation/pathstackmap.h \
    nameseparation/indexer3d.h \
    nameseparation/operators.h \
    dtextlineseparator.h \
    dslantangle.h \
    dtcm.h

LIBS += -L/urs/local/include/opencv2 -l:libopencv_core.so.2.4 -l:libopencv_highgui.so.2.4 -l:libopencv_imgproc.so.2.4 -Ilpng120/ -lpng -ltiff
LIBS += -lgsl
LIBS += -lgslcblas
QMAKE_CXXFLAGS += -fPIC -std=c++0x

OTHER_FILES += \
    nameseparation/maxflow/instances.inc
