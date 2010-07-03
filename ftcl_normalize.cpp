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

static void process_line(DataArray*image, DataArray*alpha,
			 const vector<long>&image_ptr, long wid,
			 int target);

int FitsbenchMain::ftcl_normalize_thunk_(ClientData raw, Tcl_Interp*interp,
				     int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_normalize_(objc, objv);
}

/*
 * normalize <name> <alpha> <target>
 */
int FitsbenchMain::ftcl_normalize_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 4) {
	    Tcl_AppendResult(tcl_engine_, "Usage: normalize <name> <alpha> <target>", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem*image_item = item_from_name_(objv[1]);
      if (image_item == 0) {
	    Tcl_AppendResult(tcl_engine_, "Unable to find image ",
			     Tcl_GetString(objv[1]), 0);
	    return TCL_ERROR;
      }

      DataArray*image = dynamic_cast<DataArray*> (image_item);
      if (image == 0) {
	    Tcl_AppendResult(tcl_engine_, "Item ", Tcl_GetString(objv[1]),
			     " is not a data array.", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem*alpha_item = item_from_name_(objv[2]);
      if (alpha_item == 0) {
	    Tcl_AppendResult(tcl_engine_, "Unable to find alpha image ",
			     Tcl_GetString(objv[2]), 0);
	    return TCL_ERROR;
      }

      DataArray*alpha = dynamic_cast<DataArray*> (alpha_item);
      if (alpha == 0) {
	    Tcl_AppendResult(tcl_engine_, "Item ", Tcl_GetString(objv[2]),
			     " is not a data array.", 0);
	    return TCL_ERROR;
      }

      int target = 0;
      int rc = Tcl_GetIntFromObj(tcl_engine_, objv[3], &target);
      if (rc != TCL_OK)
	    return rc;

      if (target <= 0) {
	    Tcl_AppendResult(tcl_engine_, "Target value invalid.", 0);
	    return TCL_ERROR;
      }

      vector<long> image_axes = image->get_axes();
      vector<long> image_ul = DataArray::zero_addr(image_axes.size());
      long image_wid = image_axes[1];

      pixel_iterator image_ptr (image_ul, image_axes);
      for (image_ptr.rewind() ; image_ptr.valid() ; image_ptr.incr(1,1)) {
	    process_line(image, alpha, image_ptr.value(), image_wid, target);
      }

      return TCL_OK;
}

static void process_line(DataArray*image, DataArray*alpha,
			 const vector<long>&image_ptr, long wid,
			 int target)
{
      vector<uint32_t> data_i (wid);
      vector<uint8_t>  data_a (wid);
      int has_alpha = 0;
      int rc = image->get_line(image_ptr, wid, &data_i[0], has_alpha, &data_a[0]);

      vector<uint8_t>  alpha_i(wid);
      rc = alpha->get_line(image_ptr, wid, &alpha_i[0], has_alpha);

      for (long idx = 0 ; idx < wid ; idx += 1) {
	    if (data_a[idx] == 0)
		  continue;

	    if (alpha_i[idx] == 0) {
		  data_a[idx] = 0;
		  continue;
	    }

	    data_i[idx] = data_i[idx] * target / alpha_i[idx];
      }

      image->set_line(image_ptr, wid, &data_i[0], &data_a[0]);
}
