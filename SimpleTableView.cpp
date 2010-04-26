
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

# include  <qapplication.h>
# include  "SimpleTableView.h"
# include  "DataTable.h"
# include  "qassert.h"

using namespace std;

SimpleTableView::SimpleTableView(QWidget*parent, DataTable*src, const QString&title)
: QDialog(parent)
{
      ui.setupUi(this);

      setWindowTitle(title);

	// If the table isnot configured yet, then leave it.
      if (src->table_cols() == 0) {
	    return;
      }

      size_t table_cols = src->table_cols();
      size_t table_rows = src->table_rows();
      ui.table->setColumnCount(table_cols);
      ui.table->setRowCount(table_rows);

	// Get the headings for the table.
      vector<DataTable::column_t> cols (table_cols);
      for (size_t idx = 0 ; idx < table_cols ; idx += 1) {
	    cols[idx] = src->table_col_info(idx);
	    ui.table->setHorizontalHeaderItem(idx, new QTableWidgetItem(cols[idx].heading));
      }

      for (size_t row_idx = 0 ; row_idx < table_rows ; row_idx += 1) {
	    QString row_header = QString::number(row_idx);
	    ui.table->setVerticalHeaderItem(row_idx, new QTableWidgetItem(row_header));

	    for (size_t col_idx = 0 ; col_idx < table_cols ; col_idx += 1) {

		  QString item_text;
		  switch (cols[col_idx].type) {
		      case DataTable::DT_INT32:
			item_text = QString::number(src->table_value_int32(row_idx, col_idx));
			break;
		      case DataTable::DT_STRING:
			item_text = src->table_value_string(row_idx, col_idx);
			break;
		      default:
			qassert(0);
		  }

		  ui.table->setItem(row_idx, col_idx, new QTableWidgetItem(item_text));
	    }
      }
}

SimpleTableView::~SimpleTableView()
{
}
