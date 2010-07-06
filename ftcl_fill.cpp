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

int FitsbenchMain::ftcl_fill_thunk_(ClientData raw, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      qassert(eng->tcl_engine_ == interp);
      return eng->ftcl_fill_(objc, objv);
}

/*
 * fill <work/name> <value>
 */

template <class T> static void fill_process_line(DataArray*item,
						 const vector<long>&ptr,
						 long wid, T*buf)
{
      item->set_line(ptr, wid, buf);
}

int FitsbenchMain::ftcl_fill_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 3) {
	    Tcl_AppendResult(tcl_engine_, "usage: <name> <value>", 0);
	    return TCL_ERROR;
      }

      WorkFolder::Image*item = workitem_from_name_(objv[1]);
      if (item == 0) {
	    Tcl_AppendResult(tcl_engine_, "Work Folder not found", 0);
	    return TCL_ERROR;
      }

      DataArray::type_t use_type = item->get_type();
      vector<long> axes = item->get_axes();
      vector<long> ul = DataArray::zero_addr (axes.size());

      pixel_iterator ptr (ul, axes);

      switch (item->get_type()) {
	  case DataArray::DT_VOID:
	    break;

	  case DataArray::DT_UINT8: {
		vector<uint8_t> buf (axes[0]);
		for (ptr.rewind() ; ptr.valid() ; ptr.incr(1,1)) {
		      fill_process_line(item, ptr.value(), axes[0], &buf[0]);
		}
		break;
	  }

	  case DataArray::DT_UINT32: {
		vector<uint32_t> buf (axes[0]);
		for (ptr.rewind() ; ptr.valid() ; ptr.incr(1,1)) {
		      fill_process_line(item, ptr.value(), axes[0], &buf[0]);
		}
		break;
	  }

	  default:
	    QMessageBox::information(this, tr("Not implemented"),
			       "Fill of this type not implemented yet.");
      }

      return TCL_OK;
}
