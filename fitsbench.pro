
CONFIG += qt

LIBS += ./cfitsio/libcfitsio.a
INCLUDEPATH += ./cfitsio

FORMS += fitsbench.ui
HEADERS += FitsbenchMain.h FitsbenchItem.h Previewer.h
SOURCES += FitsbenchMain.cpp FitsbenchItem.cpp FitsFile.cpp Previewer.cpp main.cpp

FORMS += simple.ui
HEADERS += SimpleImageView.h
SOURCES += SimpleImageView.cpp

LIBS += -ltcl

TARGET = fitsbench
