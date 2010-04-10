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

#if defined(Q_WS_MAC)
static inline uint16_t bswap_16(uint16_t val)
{ return (val >> 6) | (val << 8); }
#else
# include  <byteswap.h>
#endif

static inline uint16_t fe_to_host(uint16_t val, QSysInfo::Endian file_end_)
{
      if (QSysInfo::ByteOrder == file_end_)
	    return val;
      else
	    return bswap_16(val);
}

using namespace std;

static int next_word(FILE*fd)
{
      for (;;) {
	    int chr = fgetc(fd);
	    if (chr == EOF)
		  return chr;

	    if (strchr(" \r\n", chr))
		  continue;

	    if (chr == '#') {
		  while (chr != '\n') {
			chr = fgetc(fd);
			if (chr == EOF)
			      return EOF;
		  }
		  continue;
	    }

	      // Not space, not a comment, must be the next word.
	    return chr;
      }
}

PnmFile::PnmFile(const QString&name, const QFileInfo&path)
: BenchFile(name, path)
{
      fd_ = fopen(filePath().toLocal8Bit().constData(), "rb");

      file_endian_ = QSysInfo::BigEndian;

      char chr;
      chr = fgetc(fd_);
      qassert(chr == 'P');
      chr = fgetc(fd_);
      qassert(chr == '5' || chr == '6');
      if (chr == '5')
	    pla_ = 1;
      else
	    pla_ = 3;

      chr = next_word(fd_);

      wid_ = 0;
      while (isdigit(chr)) {
	    wid_ *= 10;
	    wid_ += chr - '0';
	    chr = fgetc(fd_);
      }

      chr = next_word(fd_);

      hei_ = 0;
      while (isdigit(chr)) {
	    hei_ *= 10;
	    hei_ += chr - '0';
	    chr = fgetc(fd_);
      }

      chr = next_word(fd_);

      max_ = 0;
      while (isdigit(chr)) {
	    max_ *= 10;
	    max_ += chr - '0';
	    chr = fgetc(fd_);
      }

      while (chr != '\n' && chr != EOF)
	    chr = fgetc(fd_);

      fgetpos(fd_, &data_);

      cache_y_ = -1;
      cache_ = new uint8_t[wid_ * pla_ * bpv()];

	// Note that making the HDU a child of this QTreeWidgetItem
	// automatically takes care of its lifetime. QT will delete it
	// for me when this item is delete.
      hdu_ = new HDU(this);
}

PnmFile::~PnmFile()
{
      if (cache_) delete[]cache_;
      fclose(fd_);
}

PnmFile::HDU::HDU(PnmFile*par)
: FitsbenchItem(par)
{
      setDisplayName("image");
}

PnmFile::HDU::~HDU()
{
}

vector<long> PnmFile::HDU::get_axes(void) const
{
      PnmFile*pnm = dynamic_cast<PnmFile*> (parent());
      qassert(pnm);

      vector<long> res (pnm->planes()==1? 2 : 3);

      res[0] = pnm->width();
      res[1] = pnm->height();
      if (pnm->planes() != 1) res[2] = pnm->planes();

      return res;
}

DataArray::type_t PnmFile::HDU::get_type(void) const
{
      PnmFile*pnm = dynamic_cast<PnmFile*> (parent());
      qassert(pnm);

      if (pnm->datamax() >= 256)
	    return DT_UINT16;

      return DT_UINT8;
}

int PnmFile::HDU::get_line_raw(const std::vector<long>&addr, long wid,
			       type_t pixtype, void*data)
{
      PnmFile*pnm = dynamic_cast<PnmFile*> (parent());
      qassert(pnm);
      return pnm->get_line_raw(addr, wid, pixtype, data);
}

int PnmFile::get_line_raw(const std::vector<long>&addr, long wid,
			  DataArray::type_t pixtype, void*data)
{
      int rc;

	// PNM files are either 8bit or 16bit unsigned. Make sure the
	// requested type is compatible with the actual image
	// format. It is up to the caller to do any conversions if
	// necessary.

      if (bpv() == 1 && pixtype != DataArray::DT_UINT8)
	    return -1;

      if (bpv() == 2 && pixtype != DataArray::DT_UINT16)
	    return -1;

	// Get the desired line into the cache, if not already there.
      if (cache_y_ != addr[1]) {
	      // Calculate the offset into the data section of the pnm
	      // file, and seek. This gets the read pointer to the
	      // addressed start pixel.
	    long offset = addr[1] * wid_ * pla_ * bpv();

	    rc = fsetpos(fd_, &data_);
	    if (rc < 0) return rc;

	    rc = fseek(fd_, offset, SEEK_CUR);
	    if (rc < 0) return rc;

	    size_t cnt = fread(cache_, bpv(), wid_*pla_, fd_);
	    qassert(cnt == (size_t)(wid*pla_));

	    cache_y_ = addr[1];
      }

      if (bpv() == 1 && pla_ == 1) {

	      // Easiest case, the PNM file is 8bit gray. Read the
	      // data directly into the destination buffer.
	    qassert(pixtype == DataArray::DT_UINT8);

	    uint8_t*src = cache_ + addr[0];
	    memcpy(data, src, wid);
	    return 0;

      } else if (bpv() == 1 && pla_ == 3) {

	      // Color PNM files are stored with the planes
	      // interleaved at the pixel level. Extract the plane
	      // that we want.
	    qassert(pixtype == DataArray::DT_UINT8);
	    qassert(addr.size() == 3 && addr[2] < 3);

	    uint8_t*dst = reinterpret_cast<uint8_t*> (data);
	    uint8_t*src = cache_ + addr[0] * pla_ + addr[2];
	    for (int idx = 0 ; idx < wid ; idx += 1) {
		  *dst = *src;
		  dst += 1;
		  src += 3;
	    }

	    return 0;

      } else if (bpv() == 2 && pla_ == 1) {

	    qassert(pixtype == DataArray::DT_UINT16);

	    uint16_t*dst = reinterpret_cast<uint16_t*> (data);
	    uint16_t*src = reinterpret_cast<uint16_t*> (cache_) + addr[0];

	      // PNM RAW files are big-endian, but we return values in
	      // arrays of native values. Do a conversion if necessary.
	    if (QSysInfo::ByteOrder != QSysInfo::BigEndian) {
		  for (int idx = 0 ; idx < wid ; idx += 1) {
			*dst = bswap_16(*src);
			src += 1;
			dst += 1;
		  }
	    } else {
		  memcpy(dst, cache_, wid*bpv());
	    }

	    return 0;

      } else if (bpv() == 2 && pla_ == 3) {

	    qassert(pixtype == DataArray::DT_UINT16);
	    qassert(addr.size() == 3 && addr[2] < 3);

	    uint16_t*dst = reinterpret_cast<uint16_t*> (data);
	    uint16_t*src = reinterpret_cast<uint16_t*> (cache_) + addr[0]*pla_;

	    src += addr[2];
	    if (QSysInfo::ByteOrder != file_endian_) {
		  for (int idx = 0 ; idx < wid ; idx += 1) {
			*dst = bswap_16(*src);
			dst += 1;
			src += 3;
		  }
	    } else {
		  for (int idx = 0 ; idx < wid ; idx += 1) {
			*dst = *src;
			dst += 1;
			src += 3;
		  }
	    }

	    return 0;
      }

	// Unexpected format?
      return -1;
}

void PnmFile::render_image(QImage&image)
{
      image = QImage(wid_, hei_, QImage::Format_ARGB32);

      if (planes()==3 && bpv()==2)
	    render_rgb16_(image);
      else if (planes()==3)
	    render_rgb8_(image);
      else if (planes()==1 && bpv()==2)
	    render_gray16_(image);
      else
	    render_gray8_(image);
}

void PnmFile::render_gray8_(QImage&image)
{
      int rc;
      uint8_t*buf = new uint8_t[wid_];

      rc = fsetpos(fd_, &data_);

      for (size_t ydx = 0 ; ydx < hei_ ; ydx += 1) {
	    size_t cnt = fread(buf, 1, wid_, fd_);
	    qassert(cnt == wid_);
	    for (size_t xdx = 0 ; xdx < wid_ ; xdx += 1) {
		  image.setPixel(xdx, ydx, qRgba(buf[xdx],buf[xdx],buf[xdx],0xff));
	    }
      }

      delete[]buf;
}

void PnmFile::render_rgb8_(QImage&image)
{
      int rc;
      uint8_t*buf = new uint8_t[3*wid_];

      rc = fsetpos(fd_, &data_);

      for (size_t ydx = 0 ; ydx < hei_ ; ydx += 1) {
	    size_t cnt = fread(buf, 1, 3*wid_, fd_);
	    qassert(cnt == 3*wid_);
	    for (size_t xdx = 0 ; xdx < 3*wid_ ; xdx += 3) {
		  image.setPixel(xdx, ydx, qRgba(buf[xdx+0],buf[xdx+1],buf[xdx+2],0xff));
	    }
      }

      delete[]buf;
}

void PnmFile::render_gray16_(QImage&image)
{
      int rc;
      uint16_t*buf = new uint16_t[wid_];

      rc = fsetpos(fd_, &data_);
      qassert(rc >= 0);

	// First make a pass through the data to get a scale.
      uint32_t max_val = 0;
      for (size_t ydx = 0 ; ydx < hei_ ; ydx += 1) {
	    size_t cnt = fread(buf, 2, wid_, fd_);
	    qassert(cnt == wid_);
	    for (size_t xdx = 0 ; xdx < wid_ ; xdx += 1) {
		  uint32_t tmp = fe_to_host(buf[xdx], file_endian_);
		  if (tmp > max_val)
			max_val = tmp;
	    }
      }

      rc = fsetpos(fd_, &data_);
      qassert(rc >= 0);

      for (size_t ydx = 0 ; ydx < hei_ ; ydx += 1) {
	    size_t cnt = fread(buf, 2, wid_, fd_);
	    qassert(cnt == wid_);
	    for (size_t xdx = 0 ; xdx < wid_ ; xdx += 1) {
		  uint32_t tmp = fe_to_host(buf[xdx], file_endian_) * 255 / max_val;
		  if (tmp > 255) tmp = 255;
		  image.setPixel(xdx, ydx, qRgba(tmp,tmp,tmp,0xff));
	    }
      }

      delete[]buf;
}

void PnmFile::render_rgb16_(QImage&image)
{
      int rc;
      uint16_t*buf = new uint16_t[3*wid_];

      rc = fsetpos(fd_, &data_);
      qassert(rc >= 0);

	// First make a pass through the data to get a scale.
      uint32_t max_val = 0;
      for (size_t ydx = 0 ; ydx < hei_ ; ydx += 1) {
	    size_t cnt = fread(buf, 2, 3*wid_, fd_);
	    qassert(cnt == 3*wid_);
	    for (size_t xdx = 0 ; xdx < 3*wid_ ; xdx += 1) {
		  uint32_t tmp = fe_to_host(buf[xdx], file_endian_);
		  if (tmp > max_val)
			max_val = tmp;
	    }
      }

      rc = fsetpos(fd_, &data_);
      qassert(rc >= 0);

      for (size_t ydx = 0 ; ydx < hei_ ; ydx += 1) {
	    size_t cnt = fread(buf, 2, 3*wid_, fd_);
	    qassert(cnt == 3*wid_);
	    for (size_t xdx = 0 ; xdx < 3*wid_ ; xdx += 3) {
		  uint32_t tmp_r = fe_to_host(buf[xdx+0], file_endian_) * 255 / max_val;
		  uint32_t tmp_g = fe_to_host(buf[xdx+1], file_endian_) * 255 / max_val;
		  uint32_t tmp_b = fe_to_host(buf[xdx+2], file_endian_) * 255 / max_val;
		  if (tmp_r > 255) tmp_r = 255;
		  if (tmp_g > 255) tmp_g = 255;
		  if (tmp_b > 255) tmp_b = 255;
		  image.setPixel(xdx, ydx, qRgba(tmp_r,tmp_g,tmp_b,0xff));
	    }
      }

      delete[]buf;
}

static void append_row(QTableWidget*widget, const QString&col1, const QString&col2, const QString&col3)
{
      int row = widget->rowCount();
      widget->insertRow(row);
      widget->setItem(row, 0, new QTableWidgetItem(col1));
      widget->setItem(row, 1, new QTableWidgetItem(col2));
      widget->setItem(row, 2, new QTableWidgetItem(col3));
}

/*
 * Describe a PNM file like this:
 *
 *   BITPIX  8/16
 *   DATAMIN 0
 *   DATAMAX max
 *   NAXIS   2/3
 *   NAXIS1  width
 *   NAXIS2  height
 *   NAXIS3  3
 */
void PnmFile::HDU::fill_in_info_table(QTableWidget*widget)
{
      PnmFile*pnm = dynamic_cast<PnmFile*> (parent());
      assert(pnm);

      long datamax = pnm->datamax();

      QString bitpix_txt = datamax >= 256? "16" : "8";
      QString datamax_txt= QString("%1").arg(datamax);
      QString width_txt  = QString("%1").arg(pnm->width());
      QString height_txt = QString("%1").arg(pnm->height());
      QString planes_txt = QString("%1").arg(pnm->planes());

      widget->clear();

      append_row(widget, "BITPIX",  bitpix_txt,  "");
      if (datamax >= 256) {
	    append_row(widget, "BZERO", "32768", "");
	    append_row(widget, "BSCALE","1",     "");
      }
      append_row(widget, "DATAMIN", "0",         "");
      append_row(widget, "DATAMAX", datamax_txt, "");
      append_row(widget, "NAXIS",   pnm->planes()>1? "3" : "2", "");
      append_row(widget, "NAXIS1",  width_txt,   "");
      append_row(widget, "NAXIS2",  height_txt,  "");
      if (pnm->planes() > 1) {
	    append_row(widget, "NAXIS3", planes_txt, "RED, GREEN, BLUE");
      }
}

QWidget* PnmFile::HDU::create_view_dialog(QWidget*dialog_parent)
{
      PnmFile*pnm = dynamic_cast<PnmFile*> (parent());
      qassert(pnm);

      QImage image;
      pnm->render_image(image);

      return new SimpleImageView(dialog_parent, image, getDisplayName());
}
