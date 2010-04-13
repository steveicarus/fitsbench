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


int FitsbenchMain::ftcl_bayer_decomp_thunk_(ClientData raw, Tcl_Interp*interp,
					    int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_bayer_decompose_(objc, objv);
}

/*
 * Bayer-decompose a source image (2D) into a destination image.
 *
 *   bayer_decompose <dst-red> <dst-green> <dst-blue> <src>
 */
int FitsbenchMain::ftcl_bayer_decompose_(int objc, Tcl_Obj*const objv[])
{
      uint16_t NILPIX = 0xffff;

      if (objc < 3) {
	    Tcl_AppendResult(tcl_engine_, "Usage", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem*dst_item = item_from_name_(objv[1]);
      if (dst_item) {
	    Tcl_AppendResult(tcl_engine_, "Destination already exists.", 0);
	    return TCL_ERROR;
      }

      dst_item = item_from_name_(objv[2]);
      if (dst_item) {
	    Tcl_AppendResult(tcl_engine_, "Destination already exists.", 0);
	    return TCL_ERROR;
      }

      dst_item = item_from_name_(objv[3]);
      if (dst_item) {
	    Tcl_AppendResult(tcl_engine_, "Destination already exists.", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem*src_item = item_from_name_(objv[4]);
      DataArray*src = dynamic_cast<DataArray*> (src_item);
      if (src == 0) {
	    Tcl_AppendResult(tcl_engine_, "Source is not a data array", 0);
	    return TCL_ERROR;
      }

      vector<long> src_axes = src->get_axes();
      if (src_axes.size() != 2) {
	    Tcl_AppendResult(tcl_engine_, "Source is not a 2D array", 0);
	    return TCL_ERROR;
      }

      if (src->get_type() != DataArray::DT_UINT16) {
	    Tcl_AppendResult(tcl_engine_, "Source is not 16bit unsigned", 0);
	    return TCL_ERROR;
      }

      if (src_axes[0]%2 != 0) {
	    Tcl_AppendResult(tcl_engine_, "Source is not 2x2 even.", 0);
	    return TCL_ERROR;
      }

      vector<long> dst_axes (2);
      dst_axes[0] = src_axes[0];
      dst_axes[1] = src_axes[1];

      ScratchImage*dst_red = new ScratchImage("bayer red");
      dst_red->reconfig(dst_axes, DataArray::DT_UINT16);
      ui.bench_tree->addTopLevelItem(dst_red);
      set_bench_script_name_(dst_red, Tcl_GetString(objv[1]));

      ScratchImage*dst_grn = new ScratchImage("bayer green");
      dst_grn->reconfig(dst_axes, DataArray::DT_UINT16);
      ui.bench_tree->addTopLevelItem(dst_grn);
      set_bench_script_name_(dst_grn, Tcl_GetString(objv[2]));

      ScratchImage*dst_blu = new ScratchImage("bayer blue");
      dst_blu->reconfig(dst_axes, DataArray::DT_UINT16);
      ui.bench_tree->addTopLevelItem(dst_blu);
      set_bench_script_name_(dst_blu, Tcl_GetString(objv[3]));

      uint16_t*src0 = new uint16_t[dst_axes[0]];
      uint16_t*dst0 = new uint16_t[dst_axes[0]];
      uint16_t*dst1 = new uint16_t[dst_axes[0]];
      uint16_t*dst2 = new uint16_t[dst_axes[0]];

      uint8_t*alp0 = new uint8_t[dst_axes[0]];
      uint8_t*alp1 = new uint8_t[dst_axes[0]];
      uint8_t*alp2 = new uint8_t[dst_axes[0]];

	// A typical bayer filter is this 2x2 pattern:
	//
	//      G B
	//      R G

      vector<long> src_addr(2);
      vector<long> dst_addr(2);
      src_addr[0] = 0;
      dst_addr[0] = 0;
      for (int ydx = 0 ; ydx < dst_axes[1] ; ydx += 1) {
	    int rc;
	    int has_alpha = 0;
	    src_addr[1] = ydx;
	    rc = src->get_line(src_addr, dst_axes[0], src0, has_alpha);
	    qassert(rc >= 0);
	    qassert(has_alpha == 0);

	    if (ydx%2 == 0) {
		  for (int xdx = 0 ; xdx < dst_axes[0] ; xdx += 2) {
			dst0[xdx+0] = NILPIX;
			dst1[xdx+0] = src0[xdx+0];
			dst2[xdx+0] = NILPIX;

			dst0[xdx+1] = NILPIX;
			dst1[xdx+1] = NILPIX;
			dst2[xdx+1] = src0[xdx+1];

			alp0[xdx+0] = 0x00;
			alp1[xdx+0] = 0xff;
			alp2[xdx+0] = 0x00;

			alp0[xdx+1] = 0x00;
			alp1[xdx+1] = 0x00;
			alp2[xdx+1] = 0xff;
		  }
	    } else {
		  for (int xdx = 0 ; xdx < dst_axes[0] ; xdx += 2) {
			dst0[xdx+0] = src0[xdx+0];
			dst1[xdx+0] = NILPIX;
			dst2[xdx+0] = NILPIX;

			dst0[xdx+1] = NILPIX;
			dst1[xdx+1] = src0[xdx+1];
			dst2[xdx+1] = NILPIX;

			alp0[xdx+0] = 0xff;
			alp1[xdx+0] = 0x00;
			alp2[xdx+0] = 0x00;

			alp0[xdx+1] = 0x00;
			alp1[xdx+1] = 0xff;
			alp2[xdx+1] = 0x00;
		  }
	    }

	    dst_addr[1] = ydx;
	    dst_red->set_line(dst_addr, dst_axes[0], dst0);
	    dst_grn->set_line(dst_addr, dst_axes[0], dst1);
	    dst_blu->set_line(dst_addr, dst_axes[0], dst2);

	    dst_red->set_line_alpha(dst_addr, dst_axes[0], alp0);
	    dst_grn->set_line_alpha(dst_addr, dst_axes[0], alp1);
	    dst_blu->set_line_alpha(dst_addr, dst_axes[0], alp2);
      }

      delete[]dst0;
      delete[]dst1;
      delete[]dst2;
      delete[]src0;

      return TCL_OK;
}
