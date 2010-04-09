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

#if defined(Q_WS_MAC)
static inline uint16_t bswap_16(uint16_t val)
{ return (val >> 6) | (val << 8); }
#else
# include  <byteswap.h>
#endif

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

      char chr;
      chr = fgetc(fd_);
      assert(chr == 'P');
      chr = fgetc(fd_);
      assert(chr == '5' || chr == '6');
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
      assert(pnm);

      vector<long> res (pnm->planes()==1? 2 : 3);

      res[0] = pnm->width();
      res[1] = pnm->height();
      if (pnm->planes() != 1) res[2] = pnm->planes();

      return res;
}

DataArray::type_t PnmFile::HDU::get_type(void) const
{
      PnmFile*pnm = dynamic_cast<PnmFile*> (parent());
      assert(pnm);

      if (pnm->datamax() >= 256)
	    return DT_UINT16;

      return DT_UINT8;
}

int PnmFile::HDU::get_line_raw(const std::vector<long>&addr, long wid,
			       type_t pixtype, void*data)
{
      PnmFile*pnm = dynamic_cast<PnmFile*> (parent());
      assert(pnm);
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
	    assert(cnt == (size_t)(wid*pla_));

	    cache_y_ = addr[1];
      }

      if (bpv() == 1 && pla_ == 1) {

	      // Easiest case, the PNM file is 8bit gray. Read the
	      // data directly into the destination buffer.
	    assert(pixtype == DataArray::DT_UINT8);

	    uint8_t*src = cache_ + addr[0];
	    memcpy(data, src, wid);
	    return 0;

      } else if (bpv() == 1 && pla_ == 3) {

	      // Color PNM files are stored with the planes
	      // interleaved at the pixel level. Extract the plane
	      // that we want.
	    assert(pixtype == DataArray::DT_UINT8);
	    assert(addr.size() == 3 && addr[2] < 3);

	    uint8_t*dst = reinterpret_cast<uint8_t*> (data);
	    uint8_t*src = cache_ + addr[0] * pla_ + addr[2];
	    for (int idx = 0 ; idx < wid ; idx += 1) {
		  *dst = *src;
		  dst += 1;
		  src += 3;
	    }

	    return 0;

      } else if (bpv() == 2 && pla_ == 1) {

	    assert(pixtype == DataArray::DT_UINT16);

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

	    assert(pixtype == DataArray::DT_UINT16);
	    assert(addr.size() == 3 && addr[2] < 3);

	    uint16_t*dst = reinterpret_cast<uint16_t*> (data);
	    uint16_t*src = reinterpret_cast<uint16_t*> (cache_) + addr[0]*pla_;

	    src += addr[2];
	    if (QSysInfo::ByteOrder != QSysInfo::BigEndian) {
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

      int nkeys = 6;
      if (pnm->planes() > 1) nkeys += 1;

      widget->setRowCount(nkeys);

      long datamax = pnm->datamax();

      QString bitpix_txt = datamax >= 256? "16" : "8";
      QString datamax_txt= QString("%1").arg(datamax);
      QString width_txt  = QString("%1").arg(pnm->width());
      QString height_txt = QString("%1").arg(pnm->height());
      QString planes_txt = QString("%1").arg(pnm->planes());

      widget->setItem(0, 0, new QTableWidgetItem("BITPIX"));
      widget->setItem(0, 1, new QTableWidgetItem(bitpix_txt));
      widget->setItem(0, 2, new QTableWidgetItem(""));

      widget->setItem(1, 0, new QTableWidgetItem("DATAMIN"));
      widget->setItem(1, 1, new QTableWidgetItem("0"));
      widget->setItem(1, 2, new QTableWidgetItem(""));

      widget->setItem(2, 0, new QTableWidgetItem("DATAMAX"));
      widget->setItem(2, 1, new QTableWidgetItem(datamax_txt));
      widget->setItem(2, 2, new QTableWidgetItem(""));

      widget->setItem(3, 0, new QTableWidgetItem("NAXIS"));
      widget->setItem(3, 1, new QTableWidgetItem(pnm->planes()>1? "3" : "2"));
      widget->setItem(3, 2, new QTableWidgetItem(""));

      widget->setItem(4, 0, new QTableWidgetItem("NAXIS1"));
      widget->setItem(4, 1, new QTableWidgetItem(width_txt));
      widget->setItem(4, 2, new QTableWidgetItem(""));

      widget->setItem(5, 0, new QTableWidgetItem("NAXIS2"));
      widget->setItem(5, 1, new QTableWidgetItem(height_txt));
      widget->setItem(5, 2, new QTableWidgetItem(""));

      if (pnm->planes() > 1) {
	    widget->setItem(6, 0, new QTableWidgetItem("NAXIS3"));
	    widget->setItem(6, 1, new QTableWidgetItem(planes_txt));
	    widget->setItem(6, 2, new QTableWidgetItem("RED, GREEN, BLUE"));
      }
}

QWidget* PnmFile::HDU::create_view_dialog(QWidget*dialog_parent)
{
      vector<long> axes = get_axes();

      long wid = axes[0];

      QImage image (axes[0], axes[1], QImage::Format_ARGB32);

      uint8_t*rowr = new uint8_t[wid];
      uint8_t*rowg, *rowb;
      if (axes.size() > 2) {
	    rowg = new uint8_t[wid];
	    rowb = new uint8_t[wid];
      } else {
	    rowg = rowr;
	    rowb = rowr;
      }

      vector<long> addr (axes.size());
      addr[0] = 0;
      if (addr.size() > 2) addr[2] = 0;

      for (addr[1] = 0 ; addr[1] < axes[1] ; addr[1] += 1) {

	    get_line(addr, wid, rowr);
	    if (rowg != rowr) {
		  addr[2] = 1;
		  get_line(addr, wid, rowg);
		  addr[2] = 2;
		  get_line(addr, wid, rowb);
		  addr[2] = 0;
	    }
	    for (int idx = 0 ; idx < axes[0] ; idx += 1) {
		  image.setPixel(idx, addr[1], qRgba(rowr[idx], rowg[idx], rowb[idx], 0xff));
	    }
      }

      if (rowb != rowr) delete[]rowb;
      if (rowg != rowr) delete[]rowg;
      delete[]rowr;

      return new SimpleImageView(dialog_parent, image, getDisplayName());
}
