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


int FitsbenchMain::ftcl_image_thunk_(ClientData raw, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      qassert(eng->tcl_engine_ == interp);
      return eng->ftcl_image_(objc, objv);
}


/*
 * image <name> <command> <args>...
 *
 * image <name> create <type> <axes>
 * image <name> delete
 */
int FitsbenchMain::ftcl_image_(int objc, Tcl_Obj*const objv[])
{
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
	    if (objc < 5) {
		  Tcl_AppendResult(tcl_engine_, "Usage: ",
				   "image <name> create <type> <axes>", 0);
		  return TCL_ERROR;
	    }

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

	    string type_str = Tcl_GetString(objv[3]);
	    DataArray::type_t type = DataArray::type_from_string(type_str);

	    vector<long> axes = vector_from_listobj_(objv[4]);

	    WorkFolder::Image*image = new WorkFolder::Image(folder, name);
	    image->reconfig(axes, type);

	    folder->map_folder_item(name, image);
	    return TCL_OK;

      } else {
	    Tcl_AppendResult(tcl_engine_, "Invalid subcommand: ",
			     Tcl_GetString(objv[2]), 0);
	    return TCL_ERROR;
      }

      return TCL_ERROR;
}
