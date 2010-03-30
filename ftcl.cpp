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

int FitsbenchMain::ftcl_bench_thunk_(ClientData raw, Tcl_Interp*interp,
				     int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_bench_(objc, objv);
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
		  map<string,FitsbenchItem*>::iterator cur = script_names_.find(idx_nam);
		  if (cur != script_names_.end()) {
			string text = cur->second->getDisplayName().toStdString();
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
