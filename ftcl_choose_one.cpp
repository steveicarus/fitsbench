/*
 * Copyright (c) 2010Stephen Williams (steve@icarus.com)
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
# include  "ChooseOne.h"
# include  "qassert.h"

int FitsbenchMain::ftcl_choose_one_thunk_(ClientData raw, Tcl_Interp*interp,
					  int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_choose_one_(objc, objv);
}

int FitsbenchMain::ftcl_choose_one_(int objc, Tcl_Obj*const objv[])
{
      QString message;
      QStringList choose_from;

      if (objc < 3) {
	    Tcl_AppendResult(tcl_engine_, "Usage: choose_one <text> <items>...", 0);
	    return TCL_ERROR;
      }

      message = QString(Tcl_GetString(objv[1]));

      if (objc == 3) {
	    int listc;
	    const char**listv;
	    int code = Tcl_SplitList(tcl_engine_, Tcl_GetString(objv[2]), &listc, &listv);
	    if (code != TCL_OK)
		  return code;

	    if (listc == 1) {
		  Tcl_AppendResult(tcl_engine_, Tcl_GetString(objv[2]), 0);
		  Tcl_Free((char*)listv);
		  return TCL_OK;
	    }

	    for (int idx = 0 ; idx < listc ; idx += 1)
		  choose_from << QString(listv[idx]);

	    Tcl_Free((char*)listv);
      } else {
	    for (int idx = 2 ; idx < objc ; idx += 1) {
		  QString item (Tcl_GetString(objv[idx]));
		  choose_from << item;
	    }
      }

      QString result = ChooseOne::select(this, message, choose_from);

      Tcl_AppendResult(tcl_engine_, result.toStdString().c_str(), 0);
      return TCL_OK;
}
