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

# include  "FitsbenchMain.h"
# include  "FitsbenchItem.h"
# include  <iostream>
# include  "qassert.h"

using namespace std;


int FitsbenchMain::ftcl_table_thunk_(ClientData raw, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      qassert(eng->tcl_engine_ == interp);
      return eng->ftcl_table_(objc, objv);
}

/*
 * table <name> <command> <args>...
 *
 * table <name> create <column-info>...
 * table <name> cols
 * table <name> rows
 * table <name> set <row> <col> <value>
 */
int FitsbenchMain::ftcl_table_(int objc, Tcl_Obj*const objv[])
{
      int rc;

      if (objc < 3) {
	    Tcl_AppendResult(tcl_engine_, "Usage", 0);
	    return TCL_ERROR;
      }

      QString name;
      WorkFolder*folder = workfolder_from_name_(objv[1], name);
      if (folder == 0) {
	    Tcl_AppendResult(tcl_engine_, "Cannot find folder", 0);
	    return TCL_ERROR;
      }

      string cmd = Tcl_GetString(objv[2]);
      if (cmd == "create") {

	    if (WorkFolder::WorkFits* item = folder->find_item(name)) {
		  QMessageBox::StandardButton rc = QMessageBox::question
			(0, QString("Replace File"),
			 QString("Is it OK for me to replace item %1 in folder %2?")
		           .arg(name) .arg(folder->getDisplayName()),
			 QMessageBox::Yes|QMessageBox::No,
			 QMessageBox::No);
		  if (rc != QMessageBox::Yes)
			return -1;

		  folder->unmap_folder_item(name, item);
		  delete item;
	    }

	    WorkFolder::Table*table = new WorkFolder::Table(folder, name);
	    folder->map_folder_item(name, table);

	    if (objc <= 3)
		  return TCL_OK;

	    vector<DataTable::column_t>columns;
	    for (int idx = 3 ; idx < objc ; idx += 1) {
		  int curc = 0;
		  Tcl_Obj**curv;
		  int rc = Tcl_ListObjGetElements(tcl_engine_, objv[idx],
						  &curc, &curv);
		  qassert(rc == TCL_OK);
		  qassert(curc >= 2);

		  DataTable::column_t cur;
		  cur.heading = Tcl_GetString(curv[0]);

		  string type_str = Tcl_GetString(curv[1]);
		  cur.type = DataTable::type_from_string(type_str);

		  cur.repeat = 1;
		  cur.max_elements = 132;

		  columns.push_back(cur);
	    }

	    table->create_table(columns);
	    return TCL_OK;

      } else if (cmd == "cols") {

	    WorkFolder::Table*table = folder->find_table(name);
	    if (table == 0) {
		  Tcl_AppendResult(tcl_engine_, "Cannot find table in folder", 0);
		  return TCL_ERROR;
	    }

	    Tcl_Obj*val = Tcl_NewLongObj(table->table_cols());
	    Tcl_SetObjResult(tcl_engine_, val);
	    return TCL_OK;

      } else if (cmd == "rows") {

	    WorkFolder::Table*table = folder->find_table(name);
	    if (table == 0) {
		  Tcl_AppendResult(tcl_engine_, "Cannot find table in folder", 0);
		  return TCL_ERROR;
	    }

	    Tcl_Obj*val = Tcl_NewLongObj(table->table_rows());
	    Tcl_SetObjResult(tcl_engine_, val);

	    return TCL_OK;

      } else if (cmd == "get") {

	    if (objc < 5) {
		  Tcl_AppendResult(tcl_engine_, "Missing arguments", 0);
		  return TCL_ERROR;
	    }

	    WorkFolder::Table*table = folder->find_table(name);
	    if (table == 0) {
		  Tcl_AppendResult(tcl_engine_, "Cannot find table in folder", 0);
		  return TCL_ERROR;
	    }

	    long row = 0;
	    long col = 0;
	    Tcl_GetLongFromObj(tcl_engine_, objv[3], &row);
	    Tcl_GetLongFromObj(tcl_engine_, objv[4], &col);

	    if (col >= (long)table->table_cols()) {
		  Tcl_AppendResult(tcl_engine_, "Invalid column", 0);
		  return TCL_ERROR;
	    }
	    if (row >= (long)table->table_rows()) {
		  Tcl_AppendResult(tcl_engine_, "Invalid row", 0);
		  return TCL_ERROR;
	    }

	    DataTable::column_t info = table->table_col_info(col);
	    Tcl_Obj*res = 0;
	    switch (info.type) {
		case DataTable::DT_INT32:
		  res = Tcl_NewLongObj(table->table_value_int32(row, col));
		  break;
		case DataTable::DT_DOUBLE:
		  res = Tcl_NewDoubleObj(table->table_value_double(row, col));
		  break;
		default:
		  Tcl_AppendResult(tcl_engine_, "Unsupported column type", 0);
		  return TCL_ERROR;
	    }

	    qassert(res != 0);
	    Tcl_SetObjResult(tcl_engine_, res);
	    return TCL_OK;

      } else if (cmd == "set") {

	    if (objc < 6) {
		  Tcl_AppendResult(tcl_engine_, "Missing arguments", 0);
		  return TCL_ERROR;
	    }

	    WorkFolder::Table*table = folder->find_table(name);
	    if (table == 0) {
		  Tcl_AppendResult(tcl_engine_, "Cannot find table in folder", 0);
		  return TCL_ERROR;
	    }

	    long row = 0;
	    long col = 0;
	    Tcl_GetLongFromObj(tcl_engine_, objv[3], &row);
	    Tcl_GetLongFromObj(tcl_engine_, objv[4], &col);

	    if (col >= (long)table->table_cols()) {
		  Tcl_AppendResult(tcl_engine_, "Invalid column", 0);
		  return TCL_ERROR;
	    }
	    if (row > (long)table->table_rows()) {
		  Tcl_AppendResult(tcl_engine_, "Invalid row", 0);
		  return TCL_ERROR;
	    }

	    DataTable::column_t info = table->table_col_info(col);
	    long val_long;
	    double val_double;
	    char*val_text;
	    switch (info.type) {
		case DataTable::DT_INT32:
		  Tcl_GetLongFromObj(tcl_engine_, objv[5], &val_long);
		  table->set_value_int32(row, col, val_long);
		  break;
		case DataTable::DT_DOUBLE:
		  Tcl_GetDoubleFromObj(tcl_engine_, objv[5], &val_double);
		  table->set_value_double(row, col, val_double);
		  break;
		case DataTable::DT_STRING:
		  val_text = Tcl_GetString(objv[5]);
		  rc = table->set_value_string(row, col, val_text);
		  if (rc < 0) {
			Tcl_AppendResult(tcl_engine_, "Error setting string value", 0);
			return TCL_ERROR;
		  }
		  break;
		default:
		  Tcl_AppendResult(tcl_engine_, "Unsupported column type", 0);
		  return TCL_ERROR;
	    }

	    return TCL_OK;

      } else {
	    Tcl_AppendResult(tcl_engine_, "Invalid subcommand", 0);
	    return TCL_ERROR;
      }

      return TCL_ERROR;
}
