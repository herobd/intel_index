TEMPLATE = app
CONFIG += console
CONFIG += qt

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
    pathstackmap.cpp

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
    pathstackmap.h

OTHER_FILES += \
    maxflow/instances.inc \
    vxl-1.14.0/vcl/vcl_strstream.not \
    vxl-1.14.0/vcl/vcl_rel_ops.not

LIBS += -lgsl
LIBS += -lgslcblas
LIBS += -lm
