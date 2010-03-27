
CONFIG += qt

LIBS += ./cfitsio/libcfitsio.a
INCLUDEPATH += ./cfitsio

FORMS += fitsbench.ui

HEADERS += FitsbenchMain.h FitsbenchItem.h Previewer.h
SOURCES += FitsbenchMain.cpp FitsbenchItem.cpp Previewer.cpp main.cpp

LIBS += -ltcl

TARGET = fitsbench
