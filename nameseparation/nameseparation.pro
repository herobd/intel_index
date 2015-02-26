TEMPLATE = app
CONFIG += console
CONFIG += qt
CONFIG += console debug

SOURCES += main.cpp \
    ./wordprofile.cpp \
    maxflow/maxflow.cpp \
    maxflow/graph.cpp \
    wordseparator.cpp \
    boxcleaner.cpp \
    imageaverager.cpp \
    bimage.cpp \
    bpartition.cpp \
    dimension.cpp \
    graphcut.cpp \
    gpartition.cpp \
    gimage.cpp \
    evaluate.cpp \
    angleimage.cpp \
    distancetransform.cpp \
    operators.cpp \
    blobskeleton.cpp \
    pathstackmap.cpp \
    descenderpath.cpp \
    bchunker.cpp \
    probrecognizer.cpp \
    ngramtrainingexample.cpp \
    lrmfeaturevector.cpp \
    ngramjumble.cpp \
    fft/fft8g.c \
    featurespace.cpp \
    ngrammodel.cpp \
    trainer.cpp

HEADERS += \
    ./wordprofile.h \
    maxflow/graph.h \
    maxflow/block.h \
    wordseparator.h \
    boxcleaner.h \
    Constants.h \
    imageaverager.h \
    bimage.h \
    bpartition.h \
    BPixelCollection.h \
    dimension.h \
    graphcut.h \
    gpartition.h \
    gpixelcollection.h \
    gimage.h \
    evaluate.h \
    indexer3d.h \
    distancetransform.h \
    operators.h \
    angleimage.h \
    blobskeleton.h \
    pathstackmap.h \
    descenderpath.h \
    bchunker.h \
    probrecognizer.h \
    ngramtrainingexample.h \
    lrmfeaturevector.h \
    ngramjumble.h \
    featurespace.h \
    ngrammodel.h \
    trainer.h

OTHER_FILES += \
    maxflow/instances.inc \
    vxl-1.14.0/vcl/vcl_strstream.not \
    vxl-1.14.0/vcl/vcl_rel_ops.not

LIBS += -lgsl
LIBS += -lgslcblas
LIBS += -lm
#added hr stuff
INCLUDEPATH += /home/brian/intel_index/hr/src /home/brian/intel_index/flann-1.8.4-src/src/cpp  /home/brian/boost_1_56_0 /home/brian/intel_index/kgraph-1.2-x86_64 /home/brian/intel_index/kgraph-1.2-x86_64/bin
#INCLUDEPATH += /urs/local/include
LIBS += -L/home/brian/intel_index/hr/lib/ -ldocumentproj_2013.08.30
LIBS += -L/home/brian/intel_index/kgraph-1.2-x86_64/bin/ -l:libkgraph.so
LIBS += -L/urs/local/include/opencv2 -lopencv_viz -l:libopencv_core.so.2.4
QMAKE_CXXFLAGS+= -std=c++11

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../kgraph-1.1-x86_64/bin/release/ -lkgraph
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../kgraph-1.1-x86_64/bin/debug/ -lkgraph
#else:symbian: LIBS += -lkgraph
#else:unix: LIBS += -L$$PWD/../kgraph-1.1-x86_64/bin/ -lkgraph

INCLUDEPATH += $$PWD/../kgraph-1.1-x86_64/bin
DEPENDPATH += $$PWD/../kgraph-1.1-x86_64/bin
