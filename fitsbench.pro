
CONFIG += qt

LIBS += ./sub/lib/libcfitsio.a ./sub/lib/libfftw3.a
INCLUDEPATH += ./sub/include

FORMS += fitsbench.ui

HEADERS += FitsbenchMain.h FitsbenchItem.h Previewer.h DataArray.h
SOURCES += FitsbenchMain.cpp FitsbenchItem.cpp FitsFile.cpp PnmFile.cpp ScratchImage.cpp Previewer.cpp DataArray.cpp main.cpp

FORMS += simple.ui
HEADERS += SimpleImageView.h
SOURCES += SimpleImageView.cpp

# Script command implementations...
SOURCES += ftcl.cpp ftcl_crop.cpp ftcl_fft.cpp

LIBS += -ltcl

TARGET = fitsbench
