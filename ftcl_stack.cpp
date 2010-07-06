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
# include  <iostream>
# include  "qassert.h"

using namespace std;

int FitsbenchMain::ftcl_stack_thunk_(ClientData raw, Tcl_Interp*interp,
				     int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_stack_(objc, objv);
}

static void stack_process_line(DataArray*dst, DataArray*alpha,
			       const vector<long>&dst_ptr, long wid,
			       DataArray*src, const vector<long>&src_ptr);

/*
 * Add a source image into a destination image. This stacks a single
 * image into the destination. A complete "stack" would be a sequence
 * of individual stacks. This function also uses an alpha image to
 * keep track of how many pixels are added into the stack. A separate
 * normalizing function can use this information to scale pixels to
 * complete them.
 *
 *    stack <dst> <alpha> <point> <src>
 *
 * The <dst> is an existing writable array where the source image will
 * go, and <point> is where in the image to stack. The image will be
 * cropped to fit the destination image, so for example if the <point>
 * has negative values, some bits will be chopped off.
 *
 * The <alpha> image is another array to use count the pixels added in
 * to the destination stack. Each array element where a source pixel
 * is defined will get a +1 added in. The <dst> and <alpha> arrays
 * must have the same size, although the data types make be different.
 *
 * The <point> is the location within the <dst>/<alpha> where the
 * source is written.
 *
 * The <src> is the source image.
 */
int FitsbenchMain::ftcl_stack_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 5) {
	    Tcl_AppendResult(tcl_engine_, "Usage", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem*dst_item = item_from_name_(objv[1]);
      if (dst_item == 0) {
	    Tcl_AppendResult(tcl_engine_, "Destination item ",
			     Tcl_GetString(objv[1]), " not found.", 0);
	    return TCL_ERROR;
      }

      DataArray*dst = dynamic_cast<DataArray*> (dst_item);
      if (dst == 0) {
	    Tcl_AppendResult(tcl_engine_, "Destination item ",
			     Tcl_GetString(objv[1]), " is not an array.", 0);
	    return TCL_ERROR;
      }

      vector<long> dst_axes = dst->get_axes();

      FitsbenchItem*alpha_item = item_from_name_(objv[2]);
      if (alpha_item == 0) {
	    Tcl_AppendResult(tcl_engine_, "Destination alpha item ",
			     Tcl_GetString(objv[2]), " not found.", 0);
	    return TCL_ERROR;
      }

      DataArray*alpha = dynamic_cast<DataArray*> (alpha_item);
      if (alpha == 0) {
	    Tcl_AppendResult(tcl_engine_, "Destination alpha item ",
			     Tcl_GetString(objv[2]), " is not an array.", 0);
	    return TCL_ERROR;
      }

      vector<long> alpha_axes = alpha->get_axes();
      if (alpha_axes.size() != dst_axes.size()) {
	    Tcl_AppendResult(tcl_engine_, "Destination alpha dimensionality "
			     "doesn't match dimensionality of destination array.", 0);
	    return TCL_ERROR;
      }

      for (size_t idx = 0 ; idx < alpha_axes.size() ; idx += 1) {
	    if(alpha_axes[idx] != dst_axes[idx]) {
		  Tcl_AppendResult(tcl_engine_, "Destination alpha dimensions "
				   "don't match destination dimensions.", 0);
		  return TCL_ERROR;
	    }
      }

      vector<long> dst_pnt = vector_from_listobj_(objv[3]);
      if (dst_pnt.size() != dst_axes.size()) {
	    Tcl_AppendResult(tcl_engine_, "Destination point ",
			     Tcl_GetString(objv[3]),
			     " not compatible with array.", 0);
	    return TCL_ERROR;
      }

      for (size_t idx = 0 ; idx < dst_pnt.size() ; idx += 1) {
	    if (dst_pnt[idx] >= dst_axes[idx]) {
		  Tcl_AppendResult(tcl_engine_, "Destination point ",
				   Tcl_GetString(objv[3]),
				   " is outside destination array.", 0);
		  return TCL_ERROR;
	    }
      }

      FitsbenchItem*src_item = item_from_name_(objv[4]);
      if (src_item == 0) {
	    Tcl_AppendResult(tcl_engine_, "Source item ",
			     Tcl_GetString(objv[4]), " not found.", 0);
	    return TCL_ERROR;
      }

      DataArray*src = dynamic_cast<DataArray*> (src_item);
      if (src == 0) {
	    Tcl_AppendResult(tcl_engine_, "Source item ",
			     Tcl_GetString(objv[4]), " is not an array.", 0);
	    return TCL_ERROR;
      }

      vector<long> src_axes = src->get_axes();

	// The command arguments are collected and checked. Now we
	// start the process. The data we are working with are in
	// these variables:
	//   dst      -- destination array
	//   dst_axes -- destination dimensions
	//   dst_pnt  -- destination point
	//   alpha    -- alpha array
	//   src      -- source array
	//   src_axes -- source array dimensions
	// Note that the source and destination arrays are not
	// necessarily the same shape or type.

      vector<long> dst_ul (dst_axes.size());
      for (size_t idx = 0 ; idx < dst_ul.size() ; idx += 1) {
	    if (dst_pnt[idx] <= 0)
		  dst_ul[idx] = 0;
	    else
		  dst_ul[idx] = dst_pnt[idx];
      }

      vector<long> dst_lr (dst_axes.size());
      for (size_t idx = 0 ; idx < dst_lr.size() ; idx += 1) {
	    if (idx > src_axes.size())
		  dst_lr[idx] = dst_ul[idx];
	    else if ((dst_pnt[idx]+src_axes[idx]) >= dst_axes[idx])
		  dst_lr[idx] = dst_axes[idx];
	    else
		  dst_lr[idx] = dst_pnt[idx]+src_axes[idx];
      }

      vector<long> src_ul (src_axes.size());
      qassert(src_ul.size() <= dst_axes.size());
      for (size_t idx = 0 ; idx < src_ul.size() ; idx += 1) {
	    if (dst_pnt[idx] <= 0)
		  src_ul[idx] = -dst_pnt[idx];
	    else
		  src_ul[idx] = 0;
      }

      vector<long> src_lr (src_axes.size());
      for (size_t idx = 0 ; idx < src_axes.size() ; idx += 1) {
	    src_lr[idx] = src_ul[idx];
	    src_lr[idx] += dst_lr[idx] - dst_ul[idx];
      }

      pixel_iterator dst_ptr (dst_ul, dst_lr);
      pixel_iterator src_ptr (src_ul, src_lr);
      long wid = dst_lr[0] - dst_ul[0];
      qassert(wid > 0);
      for (dst_ptr.rewind(), src_ptr.rewind()
		 ; dst_ptr.valid() && src_ptr.valid()
		 ; dst_ptr.incr(1,1), src_ptr.incr(1,1)) {
	    stack_process_line(dst, alpha,
			       dst_ptr.value(), wid,
			       src, src_ptr.value());
      }

      return TCL_OK;
}

static void stack_process_line(DataArray*dst, DataArray*alpha,
			       const vector<long>&dst_ptr, long wid,
			       DataArray*src, const vector<long>&src_ptr)
{
      vector<uint32_t> sum (wid);
      vector<uint8_t>  count(wid);
      vector<uint16_t> data(wid);
      vector<uint8_t>  data_a(wid);
      int has_alpha;
      int rc;

      rc = src  ->get_line(src_ptr, wid, &data[0],  has_alpha, &data_a[0]);
      rc = dst  ->get_line(dst_ptr, wid, &sum[0],   has_alpha, 0);
      rc = alpha->get_line(dst_ptr, wid, &count[0], has_alpha, 0);

      for (long idx = 0 ; idx < wid ; idx += 1) {
	    if (data_a[idx]) {
		  sum[idx] += data[idx];
		  count[idx] += 1;
	    }
      }

      rc = dst  ->set_line<uint32_t>(dst_ptr, wid, &sum[0]);
      rc = alpha->set_line<uint8_t> (dst_ptr, wid, &count[0]);
}
