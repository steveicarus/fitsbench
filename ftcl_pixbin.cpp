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

int FitsbenchMain::ftcl_pixbin_thunk_(ClientData raw, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      qassert(eng->tcl_engine_ == interp);
      return eng->ftcl_pixbin_(objc, objv);
}

template <class data_t, class accum_t>
void do_pixbin(ScratchImage*dst, DataArray*src,
	       const vector<long>&src_bins,
	       data_t*dst_data, accum_t*accum)
{
      vector<long> dst_axes = dst->get_axes();
      vector<long> src_axes = src->get_axes();

      uint8_t*alpha = new uint8_t[src_axes[0]];
      data_t*src_data = new data_t[src_axes[0]];

      int*count = new int[dst_axes[0]];

      vector<long> src_addr = DataArray::zero_addr(src_axes.size());
      vector<long> dst_addr = DataArray::zero_addr(dst_axes.size());

      int rc;

      for (int ydx = 0 ; ydx < dst_axes[1] ; ydx += 1) {
	    for (int idx = 0 ; idx < dst_axes[0] ; idx += 1) {
		  accum[idx] = 0;
		  count[idx] = 0;
	    }

	    for (int ybin = 0 ; ybin < src_bins[1] ; ybin += 1) {
		  src_addr[1] = ydx*src_bins[1] + ybin;

		  int has_alpha = 0;
		  rc = src->get_line(src_addr, dst_axes[0]*src_bins[0],
					 src_data, has_alpha, alpha);
		  qassert(rc >= 0);

		  for (int xdx = 0 ; xdx < dst_axes[0]*src_bins[0] ; xdx += 1) {
			if (alpha[xdx] == 0)
			      continue;
			accum[xdx / src_bins[0]] += src_data[xdx];
			count[xdx / src_bins[0]] += 1;
		  }
	    }

	    for (int idx = 0 ; idx < dst_axes[0] ; idx += 1) {
		  if (count[idx] == 0)
			continue;
		  dst_data[idx] = accum[idx] / count[idx];
	    }

	    dst_addr[1] = ydx;
	    rc = dst->set_line(dst_addr, dst_axes[0], dst_data);
	    qassert(rc >= 0);
      }

	// For now, only support 2D arrays.
      qassert(src_bins.size() == 2);

      delete[]src_data;
      delete[]alpha;
      delete[]count;
}

/*
 * The pixbin command usage is:
 *
 *    pixbin <dst> <src> <bins>
 */
int FitsbenchMain::ftcl_pixbin_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 4) {
	    Tcl_AppendResult(tcl_engine_, "Usage", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem*dst_item = item_from_name_(objv[1]);
      if (dst_item) {
	    Tcl_AppendResult(tcl_engine_, "Destination exists.", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem*src_item = item_from_name_(objv[2]);
      DataArray*src = dynamic_cast<DataArray*> (src_item);
      if (src == 0) {
	    Tcl_AppendResult(tcl_engine_, "Source is not a data array", 0);
	    return TCL_ERROR;
      }

      vector<long> src_bins = vector_from_listobj_(objv[3]);
      vector<long> src_axes = src->get_axes();

      if (src_bins.size() != src_axes.size()) {
	    Tcl_AppendResult(tcl_engine_, "Bin counts list doesn't match source dimensions.", 0);
	    return TCL_ERROR;
      }

      vector<long> dst_axes (src_axes.size());
      for (size_t idx = 0 ; idx < dst_axes.size() ; idx += 1) {
	    dst_axes[idx] = src_axes[idx] / src_bins[idx];
	    qassert(dst_axes[idx] >= 1);
      }

      DataArray::type_t dst_type = src->get_type();

      ScratchImage*dst = new ScratchImage("bin");
      dst->reconfig(dst_axes, dst_type);
      ui.bench_tree->addTopLevelItem(dst);
      set_bench_script_name_(dst, Tcl_GetString(objv[1]));

      if (dst_type == DataArray::DT_UINT8) {
	    uint8_t*data = new uint8_t[dst_axes[0]];
	    uint16_t*accum = new uint16_t[dst_axes[0]];
	    do_pixbin(dst, src, src_bins, data, accum);
	    delete[]accum;
	    delete[]data;
      } else if (dst_type == DataArray::DT_UINT16) {
	    uint16_t*data = new uint16_t[dst_axes[0]];
	    uint32_t*accum = new uint32_t[dst_axes[0]];
	    do_pixbin(dst, src, src_bins, data, accum);
	    delete[]accum;
	    delete[]data;
      } else {
	    qassert(0);
      }

      return TCL_OK;
}
