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

ScratchImage::ScratchImage(const QString&disp_name)
: FitsbenchItem(disp_name), type_(DT_VOID)
{
}

ScratchImage::~ScratchImage()
{
      delete_by_type_();
}

void ScratchImage::delete_by_type_(void)
{
      switch (type_) {
	  case DT_VOID:
	    break;
	  case DT_DOUBLE:
	    if (array_dbl_) delete[]array_dbl_;
	    array_dbl_ = 0;
	    break;
	  case DT_UINT8:
	    if (array_uint8_) delete[]array_uint8_;
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
}

std::vector<long> ScratchImage::get_axes(void) const
{
      return axes_;
}

DataArray::type_t ScratchImage::get_type(void) const
{
      return type_;
}

template <> inline double*ScratchImage::get_array_<double>(void)
{
      if (array_dbl_ == 0) {
	    array_dbl_ = new double [get_pixel_count(axes_)];
	    assert(array_dbl_);
      }
      return array_dbl_;
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
      assert(addr.size() == axes_.size());
      assert(type == type_);

      size_t off = addr[0];
      size_t dim_siz = axes_[0];
      for (size_t idx = 1 ; idx < axes_.size() ; idx += 1) {
	    off += dim_siz * addr[idx];
	    dim_siz *= axes_[idx];
      }

      switch (type) {
	  case DT_DOUBLE:
	    return do_set_line_(off, wid, reinterpret_cast<const double*>(data));
	    break;
	  default:
	    assert(0);
	    return -1;
      }
}

void ScratchImage::fill_in_info_table(QTableWidget*widget)
{
      int nkeys = 1 + axes_.size();

      widget->setRowCount(nkeys);

      widget->setItem(0, 0, new QTableWidgetItem("NAXIS"));
      widget->setItem(0, 1, new QTableWidgetItem(QString("%1").arg(axes_.size())));

      for (size_t idx = 0 ; idx < axes_.size() ; idx += 1) {
	    QString key = QString("NAXIS%1").arg(idx+1);
	    QString val = QString("%1").arg(axes_[idx]);

	    widget->setItem(1+idx, 0, new QTableWidgetItem(key));
	    widget->setItem(1+idx, 1, new QTableWidgetItem(val));
	    widget->setItem(1+idx, 2, new QTableWidgetItem(""));
      }
}

QWidget* ScratchImage::create_view_dialog(QWidget*)
{
      return 0;
}
