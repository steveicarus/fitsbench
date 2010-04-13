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
# include  "SimpleImageView.h"
# include  <QStackedWidget>
# include  <QTableWidget>
# include  <iostream>
# include  "qassert.h"

using namespace std;

FitsFile::FitsFile(const QString&name, const QFileInfo&path)
: BenchFile(name, path)
{
      int status = 0;
      fits_open_file(&fd_, filePath().toLocal8Bit().constData(), 0, &status);

      int num_hdus;
      fits_get_num_hdus(fd_, &num_hdus, &status);

      hdu_table_.resize(num_hdus);
      for (size_t idx = 0 ; idx < hdu_table_.size() ; idx += 1)
	    hdu_table_[idx] = new HDU(this, idx+1);
}

FitsFile::~FitsFile()
{
      for (size_t idx = 0 ; idx < hdu_table_.size() ; idx += 1)
	    delete hdu_table_[idx];

      int status = 0;
      fits_close_file(fd_, &status);
}

int FitsFile::movabs_hdu(int hdu_num, int&hdu_type, int&status)
{
      return fits_movabs_hdu(fd_, hdu_num, &hdu_type, &status);
}

int FitsFile::get_hdrspace(int&nkeys, int&morekeys, int&status)
{
      return fits_get_hdrspace(fd_, &nkeys, &morekeys, &status);
}

int FitsFile::read_keyn(int keynum, QString&key, QString&val, QString&com, int&status)
{
      char key_buf [FLEN_KEYWORD];
      char val_buf [FLEN_VALUE];
      char com_buf [FLEN_COMMENT];

      key_buf[0] = 0;
      val_buf[0] = 0;
      com_buf[0] = 0;
      int rc = fits_read_keyn(fd_, keynum, key_buf, val_buf, com_buf, &status);
      key = key_buf;
      val = val_buf;
      com = com_buf;
      return rc;
}

/*
 * The number of dimensions of an image. In FITS, an image can have 0
 * dimensions (a null image), 1 dimension (an array), 2 dimensions (a
 * grayscale image) or more.
 */
int FitsFile::get_img_dim(int&naxis, int&status)
{
      return fits_get_img_dim(fd_, &naxis, &status);
}

int FitsFile::get_img_size(std::vector<long>&naxes, int&status)
{
      int naxis = naxes.size();
      long*use_naxes = new long[naxis];
      int rc = fits_get_img_size(fd_, naxis, use_naxes, &status);

      for (int idx = 0 ; idx < naxis ; idx += 1)
	    naxes[idx] = use_naxes[idx];

      delete[]use_naxes;
      return rc;
}

int FitsFile::get_img_type(int&bitpix, int&status)
{
      return fits_get_img_type(fd_, &bitpix, &status);
}

int FitsFile::get_img_equivtype(int&bitpix, int&status)
{
      return fits_get_img_equivtype(fd_, &bitpix, &status);
}

void FitsFile::render_chdu(QImage&image, int ridx, int gidx, int bidx, int&status)
{
      int bitpix = 0;
      fits_get_img_equivtype(fd_, &bitpix, &status);

      int naxis = 0;
      get_img_dim(naxis, status);

      qassert(naxis >= 2);

      vector<long>naxes (naxis);
      get_img_size(naxes, status);

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

int FitsFile::get_line_chdu(const std::vector<long>&addr, long wid,
			    DataArray::type_t type, void*data,
			    int&has_alpha, uint8_t*alpha, int&status)
{
      int rc = 0;

	// Make sure the axes counts are correct. (cfitsio will not
	// check this, and instead will gleefully run past the end of
	// the fpixel array...)
      int naxes = 0;
      get_img_dim(naxes, status);
      qassert(naxes==addr.size());

      long*fpixel = new long [addr.size()];
      for (size_t idx = 0 ; idx < addr.size() ; idx += 1)
	    fpixel[idx] = addr[idx];

      int anynul = 0;
      char*nulmask = new char[wid];

      switch (type) {
	  case DataArray::DT_UINT8:
	    fits_read_pixnull(fd_, TBYTE, fpixel, wid, data, nulmask, &anynul, &status);
	    break;
	  case DataArray::DT_UINT16:
	    fits_read_pixnull(fd_, TUSHORT, fpixel, wid, data, nulmask, &anynul, &status);
	    break;

	  default:
	    QMessageBox::warning(0, "FitsFile::get_line_chdu",
				 "Unsupported data type?");
	    rc = -1;
	    break;
      }

      if (anynul) {
	    for (int idx = 0 ; idx < wid ; idx += 1) {
		  if (alpha) alpha[idx] = nulmask[idx]? 0x00 : 0xff;
		  if (nulmask[idx]) has_alpha += 1;
	    }
      } else {
	    if (alpha) memset(alpha, 0xff, wid);
	    has_alpha = 0;
      }

      delete[]nulmask;
      delete[]fpixel;
      return rc;
}


FitsFile::HDU::HDU(FitsFile*parent, int num)
: FitsbenchItem(parent), hdu_num_(num)
{
      QString name;

      int status = 0;
      int hdu_type = 0;
      parent->movabs_hdu(num, hdu_type, status);

      switch (hdu_type) {
	  case IMAGE_HDU:
	    name = QString("HDU %1 - IMAGE").arg(num);
	    break;
	  case ASCII_TBL:
	    name = QString("HDU %1 - ASCII TABLE").arg(num);
	    break;
	  case BINARY_TBL:
	    name = QString("HDU %1 - BINARY TABLE").arg(num);
	    break;
	  default:
	    name = QString("HDU %1").arg(num);
      }

      setDisplayName(name);
}

FitsFile::HDU::~HDU()
{
}

vector<long> FitsFile::HDU::get_axes(void) const
{
      FitsFile*fits = dynamic_cast<FitsFile*> (parent());
      qassert(fits);

      int status = 0;
      int naxis = 0;
      fits->get_img_dim(naxis, status);

      vector<long>res (naxis);
      fits->get_img_size(res, status);

      return res;
}

DataArray::type_t FitsFile::HDU::get_type(void) const
{
      FitsFile*fits = dynamic_cast<FitsFile*> (parent());
      qassert(fits);

      int status = 0;
      int bitpix = 0;
      fits->get_img_equivtype(bitpix, status);

      switch (bitpix) {
	  case BYTE_IMG: return DT_UINT8;
	  case SBYTE_IMG: return DT_INT8;
	  case SHORT_IMG: return DT_INT16;
	  case USHORT_IMG: return DT_UINT16;
	  case LONG_IMG: return DT_INT32;
	  case ULONG_IMG: return DT_UINT32;
	  case LONGLONG_IMG: return DT_INT64;
	  case FLOAT_IMG: return DT_FLOAT32;
	  case DOUBLE_IMG: return DT_FLOAT64;
	  default: return DT_VOID;
      }
}

int FitsFile::HDU::get_line_raw(const std::vector<long>&addr, long wid,
				type_t type, void*data,
				int&has_alpha, uint8_t*alpha)
{
      FitsFile*fits = dynamic_cast<FitsFile*> (parent());
      qassert(fits);

      if (get_type() != type)
	    return -1;
	// Fits addresses are 1-based (FORTRAN) whereas DataArray
	// addresses are zero-based.
      vector<long>fits_addr = addr;
      for (size_t idx = 0 ; idx < fits_addr.size() ; idx += 1)
	    fits_addr[idx] += 1;

      int status = 0;
      int hdu_type = 0;
      fits->movabs_hdu(hdu_num_, hdu_type, status);
      fits->get_line_chdu(fits_addr, wid, type, data, has_alpha, alpha, status);

      if (status != 0) {
	    char status_str[FLEN_STATUS];
	    fits_get_errstatus(status, status_str);
	    QMessageBox::warning(0, "FITS (cfitsio) get_line_raw", status_str);
      }

      return status==0? 0 : -1;
}

void FitsFile::HDU::fill_in_info_table(QTableWidget*widget)
{
      FitsFile*fits = dynamic_cast<FitsFile*> (parent());
      qassert(fits);

      int status = 0;
      int hdu_type = 0;
      fits->movabs_hdu(hdu_num_, hdu_type, status);

      int nkeys = 0;
      int morekeys = 0;
      fits->get_hdrspace(nkeys, morekeys, status);

      widget->setRowCount(nkeys);

      for (int idx = 0 ; idx < nkeys ; idx += 1) {
	    QString key_txt, val_txt, comment_txt;
	    fits->read_keyn(idx+1, key_txt, val_txt, comment_txt, status);
	    widget->setItem(idx, 0, new QTableWidgetItem(key_txt));
	    widget->setItem(idx, 1, new QTableWidgetItem(val_txt));
	    widget->setItem(idx, 2, new QTableWidgetItem(comment_txt));
      }
}

QWidget* FitsFile::HDU::create_view_dialog(QWidget*dialog_parent)
{
      FitsFile*fits = dynamic_cast<FitsFile*> (parent());
      qassert(fits);

      QImage image;
      int status = 0;
      int hdu_type = 0;
      fits->movabs_hdu(hdu_num_, hdu_type, status);

      int naxis = 0;
      fits->get_img_dim(naxis, status);

      switch (naxis) {
	  case 0: // NULL image
	  case 1: // vector
	    break;
	  case 2: // 2D image
	    fits->render_chdu(image, 1, 1, 1, status);
	    break;
	  case 3: // 3D image; treat is as 2D with chroma.
	    fits->render_chdu(image, 1, 2, 3, status);
	    break;
	  default: // N-dimensional data cube. Just pick a plane and
		   // render that.
	    fits->render_chdu(image, 1, 1, 1, status);
	    break;
      }

      if (status != 0) {
	    char status_str[FLEN_STATUS];
	    fits_get_errstatus(status, status_str);
	    QMessageBox::warning(0, "FITS (cfitsio) error", status_str);
      }

      return new SimpleImageView(dialog_parent, image, getDisplayName());
}
