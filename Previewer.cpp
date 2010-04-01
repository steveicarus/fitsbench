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

# include  "Previewer.h"
# include  <QString>
# include  <QStackedWidget>
# include  <QStringList>
# include  <QTableWidget>

Previewer::Previewer()
{
      table_ = 0;
      view_ = 0;
}

Previewer::~Previewer()
{
      if (table_) delete table_;
      if (view_) delete view_;
}

void Previewer::preview_into_stack(QStackedWidget*widget)
{
      if (table_ == 0) {
	    table_ = new QTableWidget(0, 3);

	    QStringList headers;
	    headers << QString("Keyword") << QString("Value") << QString("Comments");
	    table_->setHorizontalHeaderLabels(headers);

	    fill_in_info_table(table_);

	    widget->addWidget(table_);
      }

      widget->setCurrentWidget(table_);
}

void Previewer::render_into_dialog(QWidget*dialog_parent)
{
      if (view_ == 0) view_ = create_view_dialog(dialog_parent);
      if (view_ != 0) view_->show();
}
