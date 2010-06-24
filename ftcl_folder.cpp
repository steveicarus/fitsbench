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
# include  <errno.h>
# include  "qassert.h"

using namespace std;

int FitsbenchMain::ftcl_folder_thunk_(ClientData raw, Tcl_Interp*interp,
				     int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_folder_(objc, objv);
}

int FitsbenchMain::ftcl_folder_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 3) {
	    Tcl_AppendResult(tcl_engine_, "Missing subcommand.", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem*item = item_from_name_(objv[1]);
      const WorkFolder*folder = dynamic_cast<WorkFolder*> (item);
      if (folder == 0) {
	    Tcl_AppendResult(tcl_engine_, "Unable to find WorkFolder ",
			     Tcl_GetString(objv[1]), ".", 0);
	    return TCL_ERROR;
      }

      const char*subcmd = Tcl_GetString(objv[2]);
      if (subcmd == 0)
	    return TCL_ERROR;

	// The "images" subcommand returns a list of the bench names
	// that are available.
      if (strcmp(subcmd, "images") == 0) {
	    Tcl_ResetResult(tcl_engine_);
	    vector<QString> names = folder->image_names();
	    for (vector<QString>::iterator cur = names.begin()
		       ; cur != names.end() ; cur ++) {
		  Tcl_AppendElement(tcl_engine_, cur->toStdString().c_str());
	    }
	    return TCL_OK;
      }

      if (strcmp(subcmd, "tables") == 0) {
	    Tcl_ResetResult(tcl_engine_);
	    vector<QString> names = folder->table_names();
	    for (vector<QString>::iterator cur = names.begin()
		       ; cur != names.end() ; cur ++) {
		  Tcl_AppendElement(tcl_engine_, cur->toStdString().c_str());
	    }
	    return TCL_OK;
      }

      Tcl_AppendResult(tcl_engine_, "Invalid subcommand: ", subcmd, 0);
      return TCL_ERROR;
}
