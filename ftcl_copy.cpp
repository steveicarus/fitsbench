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
# include  "qassert.h"

using namespace std;

int FitsbenchMain::ftcl_copy_thunk_(ClientData raw, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      qassert(eng->tcl_engine_ == interp);
      return eng->ftcl_copy_(objc, objv);
}

/*
 * copy <work/name> <src>
 */
int FitsbenchMain::ftcl_copy_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 3) {
	    Tcl_AppendResult(tcl_engine_, "usage", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem*src_item = item_from_name_(objv[2]);
      if (src_item == 0) {
	    Tcl_AppendResult(tcl_engine_, "Source array not found", 0);
	    return TCL_ERROR;
      }

      WorkFolder::Image*item = workitem_from_name_(objv[1]);
      if (item == 0) {
	    Tcl_AppendResult(tcl_engine_, "Work Folder not found", 0);
	    return TCL_ERROR;
      }

      DataArray*src = dynamic_cast<DataArray*> (src_item);
      qassert(src);

      int rc = item->copy_from_array(src);
      if (rc < 0) {
	    Tcl_AppendResult(tcl_engine_, "Error copying array", 0);
	    return TCL_ERROR;
      }

      return TCL_OK;
}
