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
# include  <QMessageBox>
# include  <QStackedWidget>
# include  <QTableWidget>
# include  <iostream>
# include  "qassert.h"

using namespace std;

ScratchImage::ScratchImage(const QString&disp_name)
: FitsbenchItem(disp_name), type_(DT_VOID)
{
      alpha_ = 0;
}

ScratchImage::~ScratchImage()
{
      delete_by_type_();
}

void ScratchImage::delete_by_type_(void)
{
      if (alpha_) {
	    delete[]alpha_;
	    alpha_ = 0;
      }

      switch (type_) {
	  case DT_VOID:
	    break;
	  case DT_DOUBLE:
	    if (array_dbl_) delete[]array_dbl_;
	    array_dbl_ = 0;
	    break;
	  case DT_UINT8:
	    if (array_uint8_) delete[]array_uint8_;
	    array_uint8_ = 0;
	    break;
	  case DT_UINT16:
	    if (array_uint16_) delete[]array_uint16_;
	    array_uint16_ = 0;
	    break;
	  case DT_UINT32:
	    if (array_uint32_) delete[]array_uint32_;
	    array_uint32_ = 0;
	    break;
	  default:
	    assert(0);
	    break;
      }
}

	    
void ScratchImage::reconfig(const vector<long>&axes, DataArray::type_t type)
{
      delete_by_type_();
      axes_ = axes;
      type_ = type;
      switch (type_) {
	  case DT_VOID:
	    break;
	  case DT_DOUBLE:
	    array_dbl_ = 0;
	    break;
	  case DT_UINT8:
	    array_uint8_ = 0;
	    break;
	  case DT_UINT16:
	    array_uint16_ = 0;
	    break;
	  case DT_UINT32:
	    array_uint32_ = 0;
	    break;
	  default:
	    assert(0);
	    break;
      }
}

std::vector<long> ScratchImage::get_axes(void) const
{
      return axes_;
}

DataArray::type_t ScratchImage::get_type(void) const
{
      return type_;
}

size_t ScratchImage::addr_to_offset_(const vector<long>&addr) const
{
      size_t off = addr[0];
      size_t dim_siz = axes_[0];
      for (size_t idx = 1 ; idx < axes_.size() ; idx += 1) {
	    off += dim_siz * addr[idx];
	    dim_siz *= axes_[idx];
      }

      return off;
}

template <> inline double*ScratchImage::get_array_<double>(void)
{
	// Only if this really is a DOUBLE type.
      if (type_ != DT_DOUBLE)
	    return 0;

      if (array_dbl_ == 0) {
	    array_dbl_ = new double [get_pixel_count(axes_)];
	    assert(array_dbl_);
      }
      return array_dbl_;
}

template <> inline uint8_t*ScratchImage::get_array_<uint8_t>(void)
{
	// Only if this really is a DOUBLE type.
      if (type_ != DT_UINT8)
	    return 0;

      if (array_uint8_ == 0) {
	    array_uint8_ = new uint8_t [get_pixel_count(axes_)];
	    assert(array_uint8_);
      }
      return array_uint8_;
}

template <> inline uint16_t*ScratchImage::get_array_<uint16_t>(void)
{
	// Only if this really is a UINT16 type.
      if (type_ != DT_UINT16)
	    return 0;

      if (array_uint16_ == 0) {
	    array_uint16_ = new uint16_t [get_pixel_count(axes_)];
	    assert(array_uint16_);
      }
      return array_uint16_;
}

template <> inline uint32_t*ScratchImage::get_array_<uint32_t>(void)
{
	// Only if this really is a UINT32 type.
      if (type_ != DT_UINT32)
	    return 0;

      if (array_uint32_ == 0) {
	    array_uint32_ = new uint32_t [get_pixel_count(axes_)];
	    assert(array_uint32_);
      }
      return array_uint32_;
}

template <class T> int ScratchImage::do_set_line_(size_t off, long wid, const T*src)
{
      T*dst = get_array_<T>() + off;
      for (int idx = 0 ; idx < wid ; idx += 1)
	    *dst++ = *src++;
      return 0;
}

int ScratchImage::set_line_raw(const std::vector<long>&addr, long wid,
			       DataArray::type_t type, const void*data)
{
      qassert(addr.size() == axes_.size());
      qassert(type == type_);

      size_t off = addr_to_offset_(addr);

      switch (type) {
	  case DT_DOUBLE:
	    return do_set_line_(off, wid, reinterpret_cast<const double*>(data));
	  case DT_UINT8:
	    return do_set_line_(off, wid, reinterpret_cast<const uint8_t*>(data));
	  case DT_UINT16:
	    return do_set_line_(off, wid, reinterpret_cast<const uint16_t*>(data));
	  case DT_UINT32:
	    return do_set_line_(off, wid, reinterpret_cast<const uint32_t*>(data));

	  default:
	    assert(0);
	    return -1;
      }
}

int ScratchImage::set_line_alpha(const std::vector<long>&addr, long wid, const uint8_t*data)
{
      if (alpha_ == 0) {
	    size_t pixel_count = get_pixel_count(axes_);
	    alpha_ = new uint8_t[pixel_count];
	    memset(alpha_, 0xff, pixel_count);
      }

      qassert(addr.size() == axes_.size());
      if (addr[0]+wid > axes_[0])
	    return -1;

      size_t off = addr_to_offset_(addr);

      memcpy(alpha_ + off, data, wid);

      return 0;
}

template <class T> int ScratchImage::do_get_line_(size_t off, long wid, T*dst)
{
      T*src = get_array_<T>() + off;
      for (int idx = 0 ; idx < wid ; idx += 1)
	    *dst++ = *src++;
      return 0;
}

int ScratchImage::get_line_raw(const std::vector<long>&addr, long wid,
			       type_t type, void*data,
			       int&has_alpha, uint8_t*alpha)
{
      qassert(addr.size() == axes_.size());
      qassert(type == type_);

      size_t off = addr_to_offset_(addr);
      int rc = 0;

      switch (type) {
	  case DT_DOUBLE:
	    rc = do_get_line_(off, wid, reinterpret_cast<double*>(data));
	    break;
	  case DT_UINT8:
	    rc = do_get_line_(off, wid, reinterpret_cast<uint8_t*>(data));
	    break;
	  case DT_UINT16:
	    rc = do_get_line_(off, wid, reinterpret_cast<uint16_t*>(data));
	    break;
	  case DT_UINT32:
	    rc = do_get_line_(off, wid, reinterpret_cast<uint32_t*>(data));
	    break;
	  default:
	    qassert(0);
	    return -1;
      }

      if (alpha_ == 0) {
	    has_alpha = 0;
	    if (alpha) memset(alpha, 0xff, wid);
      } else {
	    uint8_t*alp = alpha_+off;
	    has_alpha = 0;
	    if (alpha) {
		  for (int idx = 0 ; idx < wid ; idx += 1) {
			*alpha = *alp++;
			if (*alpha != 0xff) has_alpha += 1;
			alpha += 1;
		  }
	    } else {
		  for (int idx = 0 ; idx < wid ; idx += 1) {
			if (*alp++ != 0xff) has_alpha += 1;
		  }
	    }
      }

      return rc;
}

static void append_row(QTableWidget*widget, const QString&col1, const QString&col2, const QString&col3)
{
      int row = widget->rowCount();
      widget->insertRow(row);
      widget->setItem(row, 0, new QTableWidgetItem(col1));
      widget->setItem(row, 1, new QTableWidgetItem(col2));
      widget->setItem(row, 2, new QTableWidgetItem(col3));
}

void ScratchImage::fill_in_info_table(QTableWidget*widget)
{
      widget->clear();

      append_row(widget, "NAXIS", QString("%1").arg(axes_.size()), "");
      for (size_t idx = 0 ; idx < axes_.size() ; idx += 1) {
	    QString key = QString("NAXIS%1").arg(idx+1);
	    QString val = QString("%1").arg(axes_[idx]);
	    append_row(widget, key, val, "");
      }

      switch (type_) {
	  case DT_UINT8:
	    append_row(widget, "BITPIX", "8", "UINT8");
	    break;
	  case DT_UINT16:
	    append_row(widget, "BITPIX", "16",    "UINT16");
	    append_row(widget, "BSCALE", "1.0",   "");
	    append_row(widget, "BZERO",  "32768", "");
	    break;
	  case DT_DOUBLE:
	    append_row(widget, "BITPIX", "-32",   "Native double");
	    break;
	  default:
	    append_row(widget, "BITPIX", "?", "");
	    break;
      }

      if (alpha_) {
	    append_row(widget, "COMMENT", "Alpha mask is present", "");
      }
}

QWidget* ScratchImage::create_view_dialog(QWidget*dialog_parent)
{
      if (const double*array = get_array_<double>())
	    return create_view_double_(dialog_parent, array);

      if (const uint8_t*array = get_array_<uint8_t>())
	    return create_view_uint8_(dialog_parent, array);

      if (const uint16_t*array = get_array_<uint16_t>())
	    return create_view_uint16_(dialog_parent, array);

      QString text = QString ("I don't know how to render this kind of scratch image");
      QMessageBox::warning(0, "ScratchImage render error", text);

      return 0;
}

QWidget* ScratchImage::create_view_double_(QWidget*dialog_parent, const double*array)
{
      if (axes_.size() != 2)
	    return 0;

      size_t pixel_count = get_pixel_count(axes_);
      double val_min = array[0];
      double val_max = array[0];
      for (size_t idx = 1 ; idx < pixel_count ; idx += 1) {
	    if (array[idx] > val_max) val_max = array[idx];
	    if (array[idx] < val_min) val_min = array[idx];
      }

      double val_scale = 1.0;
      if (val_max > val_min) val_scale = 255.5 / (val_max - val_min);

      QImage image (axes_[0], axes_[1], QImage::Format_ARGB32);

      for (size_t idx = 0 ; idx < pixel_count ; idx += 1) {
	    double val = (array[idx] - val_min) * val_scale;
	    if (val > 255) val = 255;
	    if (val < 0)   val = 0;

	    image.setPixel(idx % axes_[0], idx / axes_[0], qRgba(val, val, val, 0xff));
      }

      return new SimpleImageView(dialog_parent, image, getDisplayName());
}

QWidget* ScratchImage::create_view_uint8_(QWidget*dialog_parent, const uint8_t*array)
{
      if (axes_.size() != 2)
	    return 0;

      size_t pixel_count = get_pixel_count(axes_);

      QImage image (axes_[0], axes_[1], QImage::Format_ARGB32);

      for (size_t idx = 0 ; idx < pixel_count ; idx += 1) {
	    int val = array[idx];
	    if (val > 255) val = 255;
	    if (val < 0)   val = 0;

	    image.setPixel(idx % axes_[0], idx / axes_[0], qRgba(val, val, val, 0xff));
      }

      return new SimpleImageView(dialog_parent, image, getDisplayName());
}

QWidget* ScratchImage::create_view_uint16_(QWidget*dialog_parent, const uint16_t*array)
{
      if (axes_.size() != 2 && axes_.size() != 3)
	    return 0;

      size_t pixel_count = get_pixel_count(axes_);

      uint32_t max_val = 0;
      for (size_t idx = 0 ; idx < pixel_count ; idx += 1) {
	    if (alpha_ && alpha_[idx]==0)
		  continue;

	    if (array[idx] > max_val)
		  max_val = array[idx];
      }

      if (max_val < 4) max_val = 255;

      QImage image (axes_[0], axes_[1], QImage::Format_ARGB32);

      size_t pixel_area = axes_[0] * axes_[1];
      for (size_t idx = 0 ; idx < pixel_area ; idx += 1) {
	    uint32_t valr = array[idx] * 255 / max_val;
	    if (valr > 255) valr = 255;

	    int alphar = alpha_? alpha_[idx] : 0xff;
	    if (axes_.size() == 2) {
		  image.setPixel(idx % axes_[0], idx / axes_[0],
				 qRgba(valr, valr, valr, alphar));
	    } else {
		  qassert(axes_[2] >= 3);
		  uint32_t valg = array[1*pixel_area+idx] * 255 / max_val;
		  if (valg > 255) valg = 255;

		  int alphag = alpha_? alpha_[1*pixel_area+idx] : 0xff;

		  uint32_t valb = array[2*pixel_area+idx] * 255 / max_val;
		  if (valb > 255) valb = 255;

		  int alphab = alpha_? alpha_[2*pixel_area+idx] : 0xff;

		  int use_alpha;
		  if (alphar==0 && alphag==0 && alphab==0) {
			use_alpha = 0;
		  } else {
			use_alpha = 0xff;
			if (alphar==0) valr = 0;
			if (alphag==0) valg = 0;
			if (alphab==0) valb = 0;
		  }
		  image.setPixel(idx % axes_[0], idx / axes_[0],
				 qRgba(valr, valg, valb, use_alpha));
	    }
      }

      return new SimpleImageView(dialog_parent, image, getDisplayName());
}
