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

FitsbenchItem* FitsbenchMain::item_from_name_(Tcl_Obj*obj) const
{
      string idx_nam = Tcl_GetString(obj);
      return item_from_name_(idx_nam);
}

vector<long> FitsbenchMain::vector_from_listobj_(Tcl_Obj*obj)
{
      Tcl_Obj**axes_objv = 0;
      int axes_objc = 0;
      Tcl_ListObjGetElements(tcl_engine_, obj, &axes_objc, &axes_objv);

      vector<long> axes (axes_objc);
      for (int idx = 0 ; idx < axes_objc ; idx += 1) {
	    long tmp;
	    Tcl_GetLongFromObj(tcl_engine_, axes_objv[idx],  &tmp);
	    axes[idx] = tmp;
      }

      return axes;
}

Tcl_Obj* FitsbenchMain::listobj_from_vector_(const vector<long>&axes)
{
      Tcl_Obj*obj = Tcl_NewListObj(0, 0);

      for (size_t idx = 0 ; idx < axes.size() ; idx += 1) {
	    Tcl_Obj*cur = Tcl_NewLongObj(axes[idx]);
	    Tcl_ListObjAppendElement(tcl_engine_, obj, cur);
      }

      return obj;
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

int FitsbenchMain::ftcl_scratch_thunk_(ClientData raw, Tcl_Interp*interp,
				       int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_scratch_(objc, objv);
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
		  FitsbenchItem* item = item_from_name_(objv[idx]);

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

      FitsbenchItem*item_raw = item_from_name_(name);
      if (item_raw == 0)
	    return TCL_ERROR;

      DataArray*item = dynamic_cast<DataArray*>(item_raw);
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

	    assert(objc >= 6);

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

      Tcl_AppendResult(tcl_engine_, "Invalid subcommand: ", subcmd, 0);
      return TCL_ERROR;

}
