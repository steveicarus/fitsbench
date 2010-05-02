
CONFIG += qt

LIBS += ./sub/lib/libcfitsio.a ./sub/lib/libfftw3.a
INCLUDEPATH += ./sub/include

# On Mac OS X, we rely on a fink installation for some libraries.
macx {
  INCLUDEPATH += /sw/include
  LIBS += /sw/lib/libtiff.a
}
unix {
  LIBS += -ltiff
}

FORMS += fitsbench.ui

HEADERS += FitsbenchMain.h FitsbenchItem.h Previewer.h DataArray.h DataTable.h ObjectAutoPtr.h qassert.h
SOURCES += FitsbenchMain.cpp FitsbenchItem.cpp FitsFile.cpp ScratchImage.cpp Previewer.cpp DataArray.cpp DataTable.cpp ObjectAutoPtr.cpp main.cpp

SOURCES += PnmFile.cpp TiffFile.cpp WorkFolder.cpp

FORMS += simple.ui
HEADERS += SimpleImageView.h
SOURCES += SimpleImageView.cpp

FORMS += simple_table.ui
HEADERS += SimpleTableView.h
SOURCES += SimpleTableView.cpp

# Script command implementations...
SOURCES += ftcl.cpp ftcl_bayer.cpp ftcl_copy.cpp ftcl_crop.cpp ftcl_fft.cpp ftcl_pixbin.cpp ftcl_table.cpp

LIBS += -ltcl

TARGET = fitsbench
