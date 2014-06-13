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
    graphcut.cpp

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
    graphcut.h

OTHER_FILES += \
    maxflow/instances.inc

