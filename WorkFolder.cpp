/*
 * Copyright (c) 2010 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "FitsbenchItem.h"
# include  <QDir>
# include  <QTableWidget>
# include  <SimpleImageView.h>
# include  "qassert.h"

using namespace std;

WorkFolder::WorkFolder(const QString&name, const QDir&path)
: BenchFile(name, path.path()), work_path_(path)
{
      QStringList name_filters;
      name_filters << "*.fits";
      work_path_.setNameFilters(name_filters);

      QStringList files = work_path_.entryList();

      for (int idx = 0 ; idx < files.size() ; idx += 1) {
	    QString name = files[idx];
	    QFileInfo item_path (path, name);
	    if (name.endsWith(".fits"))
		  name.chop(5);
	    child_map_[name] = new Image(this, name, item_path);
      }
}

WorkFolder::~WorkFolder()
{
	// We do not have to delete the items in the child_map_
	// because the are QTreeWidgetItems that are children of me as
	// a QTreeWidgetItem; Qt as already deleted them for me.
}

WorkFolder::Image* WorkFolder::get_image(const QString&name)
{
      Image*&ptr = child_map_[name];

      if (ptr == 0) {
	    ptr = new Image(this, name);
      }

      return ptr;
}

/*
 * Implement the WorkFolder items using FITS files. An image is a
 * fits file with a single HDU that contains the data array.
 */
WorkFolder::Image::Image(WorkFolder*folder, const QString&name)
: FitsbenchItem(folder)
{
      fd_ = 0;
      setDisplayName(name);
}

WorkFolder::Image::Image(WorkFolder*folder, const QString&name, const QFileInfo&file)
: FitsbenchItem(folder)
{
      qassert(file.exists());
      setDisplayName(name);

      int status = 0;
      fits_open_diskfile(&fd_, file.filePath().toLocal8Bit().constData(),
			 READWRITE, &status);
}

WorkFolder::Image::~Image()
{
      if (fd_) {
	    int status = 0;
	    fits_close_file(fd_, &status);
      }
}

WorkFolder* WorkFolder::Image::folder()
{
      return dynamic_cast<WorkFolder*> (parent());
}

int WorkFolder::Image::copy_from_array(DataArray*src)
{
      int status = 0;
      if (fd_ != 0) {
	    return -1;
      }

      QFileInfo img_path (folder()->work_dir(), getDisplayName() + ".fits");
      qassert(! img_path.exists());

      QString path_str = img_path.filePath();
      fits_create_diskfile(&fd_, path_str.toLocal8Bit().constData(), &status);
      if (status != 0) {
	    show_fits_error_stack(path_str);
	    return -1;
      }

      qassert(fd_);

      DataArray::type_t src_type = src->get_type();

      int bitpix = 0;
      switch (src_type) {
	  case DT_VOID:
	    break;
	  case DT_UINT8:
	    bitpix = BYTE_IMG;
	    break;
	  case DT_UINT16:
	    bitpix = USHORT_IMG;
	    break;
      }
      qassert(bitpix != 0);

      vector<long> axes = src->get_axes();

      fits_create_img(fd_, bitpix, axes.size(), &axes[0], &status);

      int rc;
      if (src_type == DT_VOID) {
	    rc = 0;

      } else if (src_type == DT_UINT8) {
	    uint8_t*buf = new uint8_t[axes[0]];
	    rc = do_copy_lines_(src, buf, TBYTE);

      } else if (src_type == DT_UINT16) {
	    uint16_t*buf = new uint16_t[axes[0]];
	    rc = do_copy_lines_(src, buf, TUSHORT);

      } else {
	    qinternal_error("Source type not handled here.");
	    rc = -1;
      }

      return rc;
}

template<class T>int WorkFolder::Image::do_copy_lines_(DataArray*src,
						       T*buf, int datatype)
{
      vector<long> src_axes = src->get_axes();

      vector<long> dst_addr = DataArray::zero_addr(src_axes.size());
      vector<long> fits_addr (dst_addr.size());

      int status = 0;

      do {
	    int has_alpha = 0;
	    int rc = src->get_line(dst_addr, src_axes[0], buf, has_alpha, 0);
	    qassert(rc >= 0);

	    for (size_t idx = 0 ; idx < fits_addr.size() ; idx += 1)
		  fits_addr[idx] = dst_addr[idx] + 1;

	    fits_write_pix(fd_, datatype, &fits_addr[0], src_axes[0], buf, &status);

      } while (DataArray::incr(dst_addr, src_axes, 1));

      qassert(status == 0);
      if (status != 0) {
	    show_fits_error_stack("Copy lines");
      }

      fits_flush_file(fd_, &status);
      return 0;
}

void WorkFolder::Image::fill_in_info_table(QTableWidget*widget)
{
      if (fd_ == 0)
	    return;

      int status = 0;
      int nkeys = 0;
      int morekeys = 0;
      fits_get_hdrspace(fd_, &nkeys, &morekeys, &status);

      widget->setRowCount(nkeys);

      for (int idx = 0 ; idx < nkeys ; idx += 1) {
	    char key_buf [FLEN_KEYWORD];
	    char val_buf [FLEN_VALUE];
	    char com_buf [FLEN_COMMENT];
	    fits_read_keyn(fd_, idx+1, key_buf, val_buf, com_buf, &status);
	    widget->setItem(idx, 0, new QTableWidgetItem(key_buf));
	    widget->setItem(idx, 1, new QTableWidgetItem(val_buf));
	    widget->setItem(idx, 2, new QTableWidgetItem(com_buf));
      }
}

QWidget* WorkFolder::Image::create_view_dialog(QWidget*dialog_parent)
{
      if (fd_ == 0)
	    return 0;

      QImage image;
      int status = 0;
      int naxis = 0;
      fits_get_img_dim(fd_, &naxis, &status);

      switch (naxis) {
	  case 0: // NULL image
	  case 1: // vector
	    break;
	  case 2: // 2D image
	    render_chdu_(image, 1, 1, 1, status);
	    break;
	  case 3: // 3D image; treat is as 2D with chroma.
	    render_chdu_(image, 1, 2, 3, status);
	    break;
	  default: // N-dimensional data cube. Just pick a plane and
		   // render that.
	    render_chdu_(image, 1, 1, 1, status);
	    break;
      }

      if (status != 0) {
	    char status_str[FLEN_STATUS];
	    fits_get_errstatus(status, status_str);
	    QMessageBox::warning(0, "FITS (cfitsio) error", status_str);
      }

      return new SimpleImageView(dialog_parent, image, getDisplayName());
}

void WorkFolder::Image::render_chdu_(QImage&image, int ridx, int gidx, int bidx, int&status)
{
      int bitpix = 0;
      fits_get_img_equivtype(fd_, &bitpix, &status);

      int naxis = 0;
      fits_get_img_dim(fd_, &naxis, &status);

      qassert(naxis >= 2);

      vector<long>naxes (naxis);
      fits_get_img_size(fd_, naxis, &naxes[0], &status);

      qassert(ridx == 1 || naxis >= 3 && ridx <= naxes[2]);
      qassert(gidx == 1 || naxis >= 3 && gidx <= naxes[2]);
      qassert(bidx == 1 || naxis >= 3 && bidx <= naxes[2]);

      image = QImage(naxes[0], naxes[1], QImage::Format_ARGB32);

      long*fpixel = new long[naxis];
      for (int idx = 0 ; idx < naxis ; idx += 1)
	    fpixel[idx] = 1;

      if (bitpix == BYTE_IMG) {
	    unsigned char*rrow = new unsigned char [naxes[0]];
	    unsigned char*grow = new unsigned char [naxes[0]];
	    unsigned char*brow = new unsigned char [naxes[0]];
	    for (int ydx = 0 ; ydx < naxes[1] ; ydx += 1) {
		  fpixel[1] = ydx+1;
		  fpixel[2] = ridx;
		  int anynul = 0;
		  fits_read_pix(fd_, TBYTE, fpixel, naxes[0], 0, rrow, &anynul, &status);
		  fpixel[2] = gidx;
		  anynul = 0;
		  fits_read_pix(fd_, TBYTE, fpixel, naxes[0], 0, grow, &anynul, &status);
		  fpixel[2] = bidx;
		  anynul = 0;
		  fits_read_pix(fd_, TBYTE, fpixel, naxes[0], 0, brow, &anynul, &status);
		  for (int xdx = 0 ; xdx < naxes[0] ; xdx += 1) {
			image.setPixel(xdx, ydx, qRgba(rrow[xdx], grow[xdx], brow[xdx], 0xff));
		  }
	    }
	    delete[]rrow;
	    delete[]grow;
	    delete[]brow;
      } else if (bitpix == USHORT_IMG) {
	    unsigned short*rrow = new unsigned short [naxes[0]];
	    unsigned short*grow = new unsigned short [naxes[0]];
	    unsigned short*brow = new unsigned short [naxes[0]];
	    for (int ydx = 0 ; ydx < naxes[1] ; ydx += 1) {
		  fpixel[1] = ydx+1;
		  fpixel[2] = ridx;
		  int anynul = 0;
		  fits_read_pix(fd_, TUSHORT, fpixel, naxes[0], 0, rrow, &anynul, &status);
		  fpixel[2] = gidx;
		  anynul = 0;
		  fits_read_pix(fd_, TUSHORT, fpixel, naxes[0], 0, grow, &anynul, &status);
		  fpixel[2] = bidx;
		  anynul = 0;
		  fits_read_pix(fd_, TUSHORT, fpixel, naxes[0], 0, brow, &anynul, &status);
		  for (int xdx = 0 ; xdx < naxes[0] ; xdx += 1) {
			image.setPixel(xdx, ydx, qRgba(qBound(0,rrow[xdx]>>8,255),
						       qBound(0,grow[xdx]>>8,255),
						       qBound(0,brow[xdx]>>8,255),
						       0xff));
		  }
	    }
	    delete[]rrow;
	    delete[]grow;
	    delete[]brow;
      } else {
	    QString text = QString ("I don't know how to render bitpix=%1").arg(bitpix);
	    QMessageBox::warning(0, "FITS render error", text);
      }

      delete[]fpixel;
}
