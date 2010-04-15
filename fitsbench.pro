
CONFIG += qt

LIBS += ./sub/lib/libcfitsio.a ./sub/lib/libfftw3.a
INCLUDEPATH += ./sub/include

FORMS += fitsbench.ui

HEADERS += FitsbenchMain.h FitsbenchItem.h Previewer.h DataArray.h qassert.h
SOURCES += FitsbenchMain.cpp FitsbenchItem.cpp FitsFile.cpp ScratchImage.cpp Previewer.cpp DataArray.cpp main.cpp

SOURCES += PnmFile.cpp TiffFile.cpp

FORMS += simple.ui
HEADERS += SimpleImageView.h
SOURCES += SimpleImageView.cpp

# Script command implementations...
SOURCES += ftcl.cpp ftcl_bayer.cpp ftcl_crop.cpp ftcl_fft.cpp ftcl_pixbin.cpp

LIBS += -ltcl -ltiff

TARGET = fitsbench
