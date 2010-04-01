
CONFIG += qt

LIBS += ./cfitsio/libcfitsio.a
INCLUDEPATH += ./cfitsio

FORMS += fitsbench.ui
HEADERS += FitsbenchMain.h FitsbenchItem.h Previewer.h DataArray.h
SOURCES += FitsbenchMain.cpp FitsbenchItem.cpp FitsFile.cpp ScratchImage.cpp Previewer.cpp DataArray.cpp main.cpp

FORMS += simple.ui
HEADERS += SimpleImageView.h
SOURCES += SimpleImageView.cpp

SOURCES += ftcl.cpp

LIBS += -ltcl

TARGET = fitsbench
