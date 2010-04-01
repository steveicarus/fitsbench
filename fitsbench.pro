
CONFIG += qt

LIBS += ./cfitsio/libcfitsio.a
INCLUDEPATH += ./cfitsio

FORMS += fitsbench.ui
HEADERS += FitsbenchMain.h FitsbenchItem.h Previewer.h
SOURCES += FitsbenchMain.cpp FitsbenchItem.cpp FitsFile.cpp PnmFile.cpp Previewer.cpp main.cpp

FORMS += simple.ui
HEADERS += SimpleImageView.h
SOURCES += SimpleImageView.cpp

SOURCES += ftcl.cpp

LIBS += -ltcl

TARGET = fitsbench
