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

int FitsbenchMain::ftcl_crop_thunk_(ClientData raw, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_crop_(objc, objv);
}

/*
 * The crop command usage is:
 *
 *    crop <dst> <dst-axes> <src> <src-start>
 */
int FitsbenchMain::ftcl_crop_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 5) {
	    Tcl_AppendResult(tcl_engine_, "Usage", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem*dst_item = item_from_name_(objv[1]);
      if (dst_item) {
	    Tcl_AppendResult(tcl_engine_, "Destination exists.", 0);
	    return TCL_ERROR;
      }

      vector<long> dst_axes = vector_from_listobj_(objv[2]);

      FitsbenchItem*src_item = item_from_name_(objv[3]);
      DataArray*src = dynamic_cast<DataArray*> (src_item);
      if (src == 0) {
	    Tcl_AppendResult(tcl_engine_, "Source is not a data array", 0);
	    return TCL_ERROR;
      }

      vector<long> src_point = vector_from_listobj_(objv[4]);
      vector<long> src_axes = src->get_axes();

      if (src_point.size() != src_axes.size()) {
	    Tcl_AppendResult(tcl_engine_, "Source point doesn't match source dimensions.", 0);
	    return TCL_ERROR;
      }

      for (size_t idx = 0 ; idx < src_point.size() ; idx += 1) {
	    if (src_point[idx] >= src_axes[idx]) {
		  Tcl_AppendResult(tcl_engine_, "Source point is outside source array", 0);
		  return TCL_ERROR;
	    }
      }

      if (dst_axes.size() > src_axes.size()) {
	    Tcl_AppendResult(tcl_engine_, "Crop array has too many dimensions.", 0);
	    return TCL_ERROR;
      }

      for (size_t idx = 0 ; idx < dst_axes.size() ; idx += 1) {
	    if (src_point[idx] + dst_axes[idx] > src_axes[idx]) {
		  Tcl_AppendResult(tcl_engine_, "Crop array doesn't fit in source array.", 0);
		  return TCL_ERROR;
	    }
      }

      DataArray::type_t dst_type = src->get_type();

      ScratchImage*dst = new ScratchImage("crop");
      dst->reconfig(dst_axes, dst_type);
      ui.bench_tree->addTopLevelItem(dst);
      set_bench_script_name_(dst, Tcl_GetString(objv[1]));

      unsigned char*data = 0;

      switch (dst_type) {
	  case DataArray::DT_UINT8:
	    data = new unsigned char[dst_axes[0]];
	    break;
	  default:
	    assert(0);
      }

      vector<long>dst_addr = DataArray::zero_addr(dst_axes.size());

      do {
	    vector<long> src_addr = DataArray::add(src_point, dst_addr);
	    src->get_line_raw(src_addr, dst_axes[0], dst_type, data);
	    dst->set_line_raw(dst_addr, dst_axes[0], dst_type, data);
      } while (DataArray::incr(dst_addr, dst_axes, 1));

      delete[]data;

      return TCL_OK;
}
