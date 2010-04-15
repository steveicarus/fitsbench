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
# include  <QString>
# include  <QTableWidget>
# include  "qassert.h"

using namespace std;

TiffFile::TiffFile(const QString&name, const QFileInfo&path)
: BenchFile(name, path)
{
      fd_ = TIFFOpen(filePath().toLocal8Bit().constData(), "rM");

      if (fd_) {
	    hdu_table_.push_back(new HDU(this, 0));
	    while (TIFFReadDirectory(fd_)) {
		  hdu_table_.push_back(new HDU(this, TIFFCurrentDirectory(fd_)));
	    }
      }
}

TiffFile::~TiffFile()
{
      if (fd_) TIFFClose(fd_);
}


template <class T> void TiffFile::do_render_image_gray(QImage&image, uint32 wid, uint32 hei, T*buf)
{
      unsigned long scale = 255;

      if (sizeof(T) > 1) {
	    scale = 0;
	    for (uint32 ydx = 0 ; ydx < hei ; ydx += 1) {
		  int rc = TIFFReadScanline(fd_, buf, ydx, 0);
		  qassert(rc > 0);
		  if (rc < 0) return;

		  for (uint32 xdx = 0 ; xdx < wid ; xdx += 1) {
			if (buf[xdx] > scale) scale = buf[xdx];
		  }
	    }

	    if (scale < 1) scale = 1;
      }

      for (uint32 ydx = 0 ; ydx < hei ; ydx += 1) {
	    int rc = TIFFReadScanline(fd_, buf, ydx, 0);
	    qassert(rc > 0);
	    if (rc < 0) return;

	    for (uint32 xdx = 0 ; xdx < wid ; xdx += 1) {
		  unsigned long val = buf[xdx] * 255 / scale;
		  if (val > 0xff) val = 0xff;
		  image.setPixel(xdx, ydx, qRgba(val,val,val, 0xff));
	    }
      }
}

void TiffFile::render_image(QImage&image, TiffFile::HDU*ptr)
{

      tsize_t buf_len = TIFFScanlineSize(fd_);
      unsigned char*buf = new unsigned char[buf_len];

      vector<long> use_axes = ptr->get_axes();
      uint32 wid = use_axes[0];
      uint32 hei = use_axes[1];

      switch (ptr->get_type()) {
	  case DataArray::DT_UINT8:
	    do_render_image_gray(image, wid, hei, (uint8*)buf);
	    break;
	  case DataArray::DT_UINT16:
	    do_render_image_gray(image, wid, hei, (uint16*)buf);
	    break;
	  default:
	    qassert(0);
	    break;
      }

      delete[]buf;
}

int TiffFile::readScanline(tdata_t buf, uint32 row, tsample_t sample)
{
      return TIFFReadScanline(fd_, buf, row, sample);
}

tsize_t TiffFile::scanlineSize(void)
{
      return TIFFScanlineSize(fd_);
}

int TiffFile::setDirectory(tdir_t dirnum)
{
      return TIFFSetDirectory(fd_, dirnum);
}

template <> int TiffFile::getFieldDefaulted(ttag_t tag, uint16&val)
{
      return TIFFGetFieldDefaulted(fd_, tag, &val);
}

template <> int TiffFile::getFieldDefaulted(ttag_t tag, uint32&val)
{
      return TIFFGetFieldDefaulted(fd_, tag, &val);
}

TiffFile::HDU::HDU(TiffFile*parent, tdir_t dirnum)
: FitsbenchItem(parent), dirnum_(dirnum)
{
      int rc;
      setDisplayName(QString("Directory %1").arg(dirnum));

      uint16 samples_per_pixel;
      rc = parent->getFieldDefaulted(TIFFTAG_SAMPLESPERPIXEL, samples_per_pixel);

      uint32 wid, hei;
      rc = parent->getFieldDefaulted(TIFFTAG_IMAGEWIDTH, wid);
      rc = parent->getFieldDefaulted(TIFFTAG_IMAGELENGTH, hei);

      axes_ = vector<long> ( samples_per_pixel > 1? 3 : 2 );
      axes_[0] = wid;
      axes_[1] = hei;
      if (samples_per_pixel > 1)
	    axes_[2] = samples_per_pixel;

      uint16 bits_per_sample;
      rc = parent->getFieldDefaulted(TIFFTAG_BITSPERSAMPLE, bits_per_sample);

      if (bits_per_sample <= 8)
	    type_ = DT_UINT8;
      else if (bits_per_sample <= 16)
	    type_ = DT_UINT16;
      else if (bits_per_sample <= 32)
	    type_ = DT_UINT32;
      else
	    type_ = DT_UINT64;

      cache_y_ = 0;
      cache_ = 0;
}

TiffFile::HDU::~HDU()
{
      delete[]cache_;
}

std::vector<long> TiffFile::HDU::get_axes(void) const
{
      return axes_;
}

DataArray::type_t TiffFile::HDU::get_type(void) const
{
      return type_;
}

template <class T> int TiffFile::HDU::get_line_buf(long x, long wid, T*dst)
{
      const T*src = x + reinterpret_cast<const T*> (cache_);

      for (long idx = 0 ; idx < wid ; idx += 1)
	    *dst++ = *src++;

      return 0;
}

int TiffFile::HDU::get_line_raw(const std::vector<long>&addr, long wid,
				type_t pixtype, void*data,
				int&has_alpha, uint8_t*alpha)
{
      TiffFile*tif = dynamic_cast<TiffFile*> (parent());
      qassert(tif);
      qassert(pixtype == type_);

      tif->setDirectory(dirnum_);
      if (cache_==0) {
	    tsize_t cache_len = tif->scanlineSize();
	    cache_ = new unsigned char[cache_len];
	    cache_y_ = -1;
      }

      if (cache_y_ != addr[1]) {
	    cache_y_ = addr[1];
	    tif->readScanline(cache_, addr[1], 0);
      }

      has_alpha = 0;
      if (alpha) memset(alpha, 0xff, wid);

      switch (type_) {
	  case DT_UINT8:
	    return get_line_buf(addr[0], wid, reinterpret_cast<uint8_t*>(data));
	  case DT_UINT16:
	    return get_line_buf(addr[0], wid, reinterpret_cast<uint16_t*>(data));
	  default:
	    qassert(0);
	    return -1;
      }
}

static void append_row(QTableWidget*widget, const QString&col1, const QString&col2, const QString&col3)
{
      int row = widget->rowCount();
      widget->insertRow(row);
      widget->setItem(row, 0, new QTableWidgetItem(col1));
      widget->setItem(row, 1, new QTableWidgetItem(col2));
      widget->setItem(row, 2, new QTableWidgetItem(col3));
}

void TiffFile::HDU::fill_in_info_table(QTableWidget*widget)
{
      TiffFile*tif = dynamic_cast<TiffFile*> (parent());
      qassert(tif);

      widget->clear();

      append_row(widget, "NAXIS", QString("%1").arg(axes_.size()), "");
      for (size_t idx = 0 ; idx < axes_.size() ; idx += 1) {
	    append_row(widget, QString("NAXIS%1").arg(idx+1),
		       QString("%1").arg(axes_[idx]), "");
      }

      switch (type_) {
	  case DT_UINT8:
	    append_row(widget, "BITPIX", "8", "");
	    break;
	  case DT_UINT16:
	    append_row(widget, "BITPIX", "16", "");
	    append_row(widget, "BZERO",  "32768", "");
	    append_row(widget, "BSCALE", "1", "");
	    break;
	  default:
	    break;
      }
}

QWidget*TiffFile::HDU::create_view_dialog(QWidget*dialog_parent)
{
      TiffFile*tif = dynamic_cast<TiffFile*> (parent());
      qassert(tif);

      tif->setDirectory(dirnum_);

      QImage image(axes_[0], axes_[1], QImage::Format_ARGB32);
      tif->render_image(image, this);

      return new SimpleImageView(dialog_parent, image, getDisplayName());
}
