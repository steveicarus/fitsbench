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
: FitsbenchItem(disp_name)
{
}

ScratchImage::~ScratchImage()
{
}

void ScratchImage::reconfig(const vector<long>&axes, DataArray::type_t type)
{
      axes_ = axes;
      type_ = type;
}

std::vector<long> ScratchImage::get_axes(void) const
{
      return axes_;
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
