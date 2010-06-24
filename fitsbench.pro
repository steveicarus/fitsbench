
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

HEADERS += FitsbenchMain.h FitsbenchItem.h Previewer.h DataArray.h DataTable.h ObjectAutoPtr.h fits_helpers.h qassert.h
SOURCES += FitsbenchMain.cpp FitsbenchItem.cpp Previewer.cpp DataArray.cpp DataTable.cpp ObjectAutoPtr.cpp fits_helpers.cpp main.cpp

SOURCES += FitsFile.cpp PnmFile.cpp TiffFile.cpp ScratchImage.cpp WorkFolder.cpp

FORMS += simple.ui
HEADERS += SimpleImageView.h
SOURCES += SimpleImageView.cpp

FORMS += simple_table.ui
HEADERS += SimpleTableView.h
SOURCES += SimpleTableView.cpp

FORMS += choose_one.ui
HEADERS += ChooseOne.h
SOURCES += ChooseOne.cpp

# Script command implementations...
SOURCES += ftcl.cpp ftcl_bayer.cpp ftcl_bench.cpp ftcl_choose_one.cpp ftcl_copy.cpp ftcl_crop.cpp ftcl_define_action.cpp ftcl_fft.cpp ftcl_folder.cpp ftcl_normalize.cpp ftcl_pixbin.cpp ftcl_stack.cpp ftcl_table.cpp

LIBS += -ltcl

TARGET = fitsbench
