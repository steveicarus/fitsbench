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
}

Previewer::~Previewer()
{
}

void Previewer::preview_into_stack(QStackedWidget*widget)
{
      if (table_.ptr() == 0) {
	    QTableWidget*tmp = new QTableWidget(0, 3);

	    QStringList headers;
	    headers << QString("Keyword") << QString("Value") << QString("Comments");
	    tmp->setHorizontalHeaderLabels(headers);

	    fill_in_info_table(tmp);

	    widget->addWidget(tmp);
	    table_ = tmp;
      }

      widget->setCurrentWidget(table_.ptr());
}

void Previewer::render_into_dialog(QWidget*dialog_parent)
{
      if (view_.ptr() == 0) {
	    view_ = create_view_dialog(dialog_parent);
      }
      if (view_.ptr() != 0) view_.ptr()->show();
}

void Previewer::preview_view_changed(void)
{
      view_.destroy();
}
