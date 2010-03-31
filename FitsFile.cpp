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
# include  <assert.h>

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

void FitsFile::render_chdu(QImage&image, int ridx, int gidx, int bidx, int&status)
{
      int bitpix = 0;
      fits_get_img_equivtype(fd_, &bitpix, &status);

      int naxis = 0;
      get_img_dim(naxis, status);

      assert(naxis >= 2);

      vector<long>naxes (naxis);
      get_img_size(naxes, status);

      assert(ridx == 1 || naxis >= 3 && ridx <= naxes[2]);
      assert(gidx == 1 || naxis >= 3 && gidx <= naxes[2]);
      assert(bidx == 1 || naxis >= 3 && bidx <= naxes[2]);

      image = QImage(naxes[0], naxes[1], QImage::Format_ARGB32);

      long*fpixel = new long[naxis];
      for (int idx = 0 ; idx < naxis ; idx += 1)
	    fpixel[idx] = 1;

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

      delete[]fpixel;
}

FitsFile::HDU::HDU(FitsFile*parent, int num)
: FitsbenchItem(parent), hdu_num_(num)
{
      view_ = 0;
      preview_ = 0;
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
      if (view_) delete view_;
      if (preview_) delete preview_;
}

vector<long> FitsFile::HDU::get_axes(void) const
{
      FitsFile*fits = dynamic_cast<FitsFile*> (parent());
      assert(fits);

      int status = 0;
      int naxis = 0;
      fits->get_img_dim(naxis, status);

      vector<long>res (naxis);
      fits->get_img_size(res, status);

      return res;
}

void FitsFile::HDU::preview_into_stack(QStackedWidget*wstack)
{
      if (preview_ == 0) {
	    FitsFile*fits = dynamic_cast<FitsFile*> (parent());
	    assert(fits);

	    int status = 0;
	    int hdu_type = 0;
	    fits->movabs_hdu(hdu_num_, hdu_type, status);

	    int nkeys = 0;
	    int morekeys = 0;
	    fits->get_hdrspace(nkeys, morekeys, status);

	    QStringList headers;
	    headers << QString("Keyword") << QString("Value") << QString("Comments");
	    preview_ = new QTableWidget(nkeys, 3);
	    preview_->setHorizontalHeaderLabels(headers);

	    for (int idx = 0 ; idx < nkeys ; idx += 1) {
		  QString key_txt, val_txt, comment_txt;
		  fits->read_keyn(idx+1, key_txt, val_txt, comment_txt, status);
		  preview_->setItem(idx, 0, new QTableWidgetItem(key_txt));
		  preview_->setItem(idx, 1, new QTableWidgetItem(val_txt));
		  preview_->setItem(idx, 2, new QTableWidgetItem(comment_txt));
	    }
	    wstack->addWidget(preview_);
      }

      wstack->setCurrentWidget(preview_);
}

void FitsFile::HDU::render_into_dialog(QWidget*dialog_parent)
{
      if (view_) {
	    view_->show();
	    return;
      }

      FitsFile*fits = dynamic_cast<FitsFile*> (parent());
      assert(fits);

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

      view_ = new SimpleImageView(dialog_parent, image, getDisplayName());
      view_->show();
}