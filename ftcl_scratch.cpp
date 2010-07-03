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

int FitsbenchMain::ftcl_scratch_thunk_(ClientData raw, Tcl_Interp*interp,
				       int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_scratch_(objc, objv);
}

int FitsbenchMain::ftcl_scratch_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 2) {
	    Tcl_AppendResult(tcl_engine_, "Missing subcommand.", 0);
	    return TCL_ERROR;
      }

      const char*subcmd = Tcl_GetString(objv[1]);
      if (subcmd == 0)
	    return TCL_ERROR;

      if (strcmp(subcmd, "new") == 0) {
	      // The format of the "new" subcommand is:
	      //   scratch new <name> <display-name> <type> <axes-list>

	    if (objc < 6) {
		  Tcl_AppendResult(tcl_engine_, "Usage: "
				   "scratch new <name> <display> <type> <axes>", 0);
		  return TCL_ERROR;
	    }
	    qassert(objc >= 6);

	    QString name = Tcl_GetString(objv[2]);
	    QString disp_name = Tcl_GetString(objv[3]);
	    string type_str = Tcl_GetString(objv[4]);
	    DataArray::type_t type = DataArray::type_from_string(type_str);

	    vector<long> axes = vector_from_listobj_(objv[5]);

	    ScratchImage*item = new ScratchImage(disp_name);
	    ui.bench_tree->addTopLevelItem(item);

	    item->reconfig(axes, type);
	    set_bench_script_name_(item, name);

	    return TCL_OK;
      }

      if (strcmp(subcmd, "delete") == 0) {
	      // The format of the "delete" subcommand is:
	      //   scratch delete <name>

	    if (objc < 3) {
		  Tcl_AppendResult(tcl_engine_, "Usage: "
				   "scratch delete <name>", 0);
		  return TCL_ERROR;
	    }
	    qassert(objc >= 3);

	    FitsbenchItem* item = item_from_name_(objv[2]);
	    if (item == 0) {
		  Tcl_AppendResult(tcl_engine_, "Item ",
				   Tcl_GetString(objv[2]),
				   " not found.", 0);
		  return TCL_ERROR;
	    }

	    ScratchImage*scr = dynamic_cast<ScratchImage*> (item);
	    if (scr == 0) {
		  Tcl_AppendResult(tcl_engine_, "Item ",
				   Tcl_GetString(objv[2]),
				   " is not a scratch image.", 0);
		  return TCL_ERROR;
	    }

	    bench_tree_delete_item_(item);

	    return TCL_OK;
      }

      Tcl_AppendResult(tcl_engine_, "Invalid subcommand: ", subcmd, 0);
      return TCL_ERROR;

}
