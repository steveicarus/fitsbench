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
# include  <assert.h>

using namespace std;

FitsbenchItem* FitsbenchMain::item_from_name_(const string&name) const
{
      map<string,FitsbenchItem*>::const_iterator cur = script_names_.find(name);

      if (cur == script_names_.end()) return 0;
      else return cur->second;
}

int FitsbenchMain::ftcl_bench_thunk_(ClientData raw, Tcl_Interp*interp,
				     int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_bench_(objc, objv);
}

int FitsbenchMain::ftcl_axes_thunk_(ClientData raw, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_axes_(objc, objv);
}

int FitsbenchMain::ftcl_bench_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 2) {
	    Tcl_AppendResult(tcl_engine_, "Missing subcommand.", 0);
	    return TCL_ERROR;
      }

      const char*subcmd = Tcl_GetString(objv[1]);
      if (subcmd == 0)
	    return TCL_ERROR;

	// The "names" subcommand returns a list of the bench names
	// that are available.
      if (strcmp(subcmd, "names") == 0) {
	    Tcl_ResetResult(tcl_engine_);
	    for (map<string,FitsbenchItem*>::iterator cur = script_names_.begin()
		       ; cur != script_names_.end() ; cur ++) {
		  Tcl_AppendElement(tcl_engine_, cur->first.c_str());
	    }
	    return TCL_OK;
      }

      if (strcmp(subcmd, "display_text") == 0) {
	    Tcl_ResetResult(tcl_engine_);
	    for (int idx = 2 ; idx < objc ; idx += 1) {
		  string idx_nam = Tcl_GetString(objv[idx]);
		  FitsbenchItem* item = item_from_name_(idx_nam);

		  if (item) {
			string text = item->getDisplayName().toStdString();
			Tcl_AppendElement(tcl_engine_, text.c_str());
		  } else {
			Tcl_AppendElement(tcl_engine_, string("").c_str());
		  }
	    }
	    return TCL_OK;
      }

      Tcl_AppendResult(tcl_engine_, "Invalid subcommand: ", subcmd, 0);
      return TCL_ERROR;
}

int FitsbenchMain::ftcl_axes_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 2) {
	    Tcl_AppendResult(tcl_engine_, "Missing name.", 0);
	    return TCL_ERROR;
      }

      const char*name = Tcl_GetString(objv[1]);
      if (name == 0)
	    return TCL_ERROR;

      FitsbenchItem*item = item_from_name_(name);
      if (item == 0)
	    return TCL_ERROR;

      vector<long> axes = item->get_axes();

      Tcl_Obj*res = Tcl_NewObj();
      for (size_t idx = 0 ; idx < axes.size() ; idx += 1) {
	    Tcl_Obj*val = Tcl_NewLongObj(axes[idx]);
	    Tcl_ListObjAppendElement(tcl_engine_, res, val);
      }

      Tcl_SetObjResult(tcl_engine_, res);

      return TCL_OK;
}
