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
# include  <fftw3.h>
# include  "qassert.h"

using namespace std;

int FitsbenchMain::ftcl_phase_corr_thunk_(ClientData raw, Tcl_Interp*interp,
				       int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      qassert(eng->tcl_engine_ == interp);
      return eng->ftcl_phase_corr_(objc, objv);
}

template<class src_t> static void do_get_complex_array(const std::vector<long>&axes, fftw_complex*dst, DataArray*src, const std::vector<long>&src_pnt)
{
      vector<src_t>   src_buf   (axes[0]);
      vector<uint8_t> alpha_buf (axes[0]);

      size_t count = DataArray::get_pixel_count(axes);
      vector<long> src_ptr = src_pnt;
      vector<long> src_ref = src->get_axes();

      for (size_t count_idx = 0 ; count_idx < count ; count_idx += axes[0]) {
	    int has_alpha = 0;
	    int rc = src->get_line(src_ptr, axes[0], &src_buf[0],
				   has_alpha, &alpha_buf[0]);
	    qassert(rc >= 0);
	    for (long idx = 0 ; idx < axes[0] ; idx += 1) {
		  dst[0][0] = alpha_buf[idx]? src_buf[idx] : 0.0;
		  dst[0][1] = 0.0;
		  dst += 1;
	    }
	    DataArray::incr(src_ptr, src_ref, 1);
      }
}

static void get_complex_array(const std::vector<long>& axes, fftw_complex*dst, DataArray*src, const std::vector<long>&src_pnt)
{
      switch (src->get_type()) {
	  case DataArray::DT_UINT8:
	    do_get_complex_array<uint8_t>(axes, dst, src, src_pnt);
	    break;
	  case DataArray::DT_INT8:
	    do_get_complex_array<int8_t>(axes, dst, src, src_pnt);
	    break;
	  case DataArray::DT_UINT16:
	    do_get_complex_array<uint16_t>(axes, dst, src, src_pnt);
	    break;
	  case DataArray::DT_INT16:
	    do_get_complex_array<int16_t>(axes, dst, src, src_pnt);
	    break;
	  default: {
		qassert(0);
		break;
	  }
      }
}

int FitsbenchMain::ftcl_phase_corr_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 7) {
	    Tcl_AppendResult(tcl_engine_, "Usage: phase_correlate "
			     "<dst> {axes} <src1> {point> <src2> {point}", 0);
	    return TCL_ERROR;
      }

      const char*dst_name = Tcl_GetString(objv[1]);
      if (dst_name == 0)
	    return TCL_ERROR;

      vector<long> dst_axes;
      vector<long> src1_pnt;
      vector<long> src2_pnt;

	// Detect an axes list argument that defines the axes of the
	// destination array.
      const char*tmp = Tcl_GetString(objv[2]);
      if (strcmp(tmp, "-") != 0)
	    dst_axes = vector_from_listobj_(objv[2]);

      const char*src1_name = Tcl_GetString(objv[3]);
      if (src1_name == 0)
	    return TCL_ERROR;

	// Detect an optional source point
      tmp = Tcl_GetString(objv[4]);
      if (strcmp(tmp, "-") != 0) 
	    src1_pnt = vector_from_listobj_(objv[4]);

      const char*src2_name = Tcl_GetString(objv[5]);
      if (src2_name == 0)
	    return TCL_ERROR;

	// Detect an optional source point
      tmp = Tcl_GetString(objv[6]);
      if (strcmp(tmp, "-") != 0)
	    src2_pnt = vector_from_listobj_(objv[6]);

      FitsbenchItem* dst_item = item_from_name_(dst_name);

      FitsbenchItem* src1_item = item_from_name_(src1_name);
      if (src1_item == 0) {
	    Tcl_AppendResult(tcl_engine_, "Image ", src1_name, " not found.", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem* src2_item = item_from_name_(src2_name);
      if (src1_item == 0) {
	    Tcl_AppendResult(tcl_engine_, "Image ", src2_name, " not found.", 0);
	    return TCL_ERROR;
      }

      DataArray*dst = dynamic_cast<DataArray*>(dst_item);

      DataArray*src1 = dynamic_cast<DataArray*>(src1_item);
      if (src1 == 0) {
	    Tcl_AppendResult(tcl_engine_, "Item ", src1_name, " is not a data array.", 0);
	    return TCL_ERROR;
      }

      DataArray*src2 = dynamic_cast<DataArray*>(src2_item);
      if (src2 == 0) {
	    Tcl_AppendResult(tcl_engine_, "Item ", src2_name, " is not a data array.", 0);
	    return TCL_ERROR;
      }

      vector<long> src1_axes = src1->get_axes();
      vector<long> src2_axes = src2->get_axes();

	// If the destination image exists, make sure it is a
	// DataArray, and get the dimensions from the array. We will
	// use those dimensions for the target. If the destination
	// does not exist, then create it using the dimensions that
	// the user specified.
      qassert(dst || !dst_item);

      if (dst) {
	    qassert(dst_axes.size() == 0);
	    dst_axes = dst->get_axes();
      } else {
	    qassert(dst_axes.size() != 0);
	    QString dst_disp = QString("phase_correlate(%1, %2)") .arg(src1_name) .arg(src2_name);
	    ScratchImage*item = new ScratchImage(dst_disp);
	    ui.bench_tree->addTopLevelItem(item);
	    set_bench_script_name_(item, dst_name);

	    item->reconfig(dst_axes, DataArray::DT_DOUBLE);

	    dst_item = item;
	    dst = item;
      }

      qassert(dst_axes == dst->get_axes());

	// Make sure the destination array fits within the source array.
      qassert(dst_axes.size() <= src1_axes.size());
      qassert(dst_axes.size() <= src2_axes.size());
      for (size_t idx = 0 ; idx < dst_axes.size() ; idx += 1) {
	      // XXXX This is too restrictive!
	    qassert(dst_axes[idx] = src1_axes[idx]);
	    qassert(dst_axes[idx] = src2_axes[idx]);
      }

	// The pixel_count is the size of the destination array. This
	// may be smaller then the source array, so be careful.
      size_t dst_pixel_count = DataArray::get_pixel_count(dst_axes);

      fftw_complex*src1_array = (fftw_complex*)fftw_malloc(dst_pixel_count * sizeof(fftw_complex));
      fftw_complex*src2_array = (fftw_complex*)fftw_malloc(dst_pixel_count * sizeof(fftw_complex));
      fftw_complex*dst_array  = (fftw_complex*)fftw_malloc(dst_pixel_count * sizeof(fftw_complex));

      qassert(src1_array);
      qassert(src2_array);
      qassert(dst_array);

      fftw_plan plan1 = fftw_plan_dft_1d(dst_pixel_count, src1_array, src1_array,
					 FFTW_FORWARD, FFTW_ESTIMATE);
      fftw_plan plan2 = fftw_plan_dft_1d(dst_pixel_count, src2_array, src2_array,
					 FFTW_FORWARD, FFTW_ESTIMATE);
      fftw_plan pland = fftw_plan_dft_1d(dst_pixel_count, dst_array, dst_array,
					 FFTW_BACKWARD, FFTW_ESTIMATE);

      if (src1_pnt.size() == 0)
	    src1_pnt = DataArray::zero_addr(src1_axes.size());
      if (src2_pnt.size() == 0)
	    src2_pnt = DataArray::zero_addr(src2_axes.size());

	// The get_data_array method assumes that the starting point
	// in the source array is 0..0 for the dimensions of the
	// source array. This allows the src array to have more
	// dimensions and those dimensions to be non-zero.
      for (size_t idx = 0 ; idx < dst_axes.size() ; idx += 1) {
	    qassert(src1_pnt[idx] == 0);
	    qassert(src2_pnt[idx] == 0);
      }

      get_complex_array(dst_axes, src1_array, src1, src1_pnt);
      get_complex_array(dst_axes, src2_array, src2, src2_pnt);

      fftw_execute(plan1);
      fftw_execute(plan2);

      for (size_t idx = 0 ; idx < dst_pixel_count ; idx += 1) {
	    fftw_complex*cur1 = src1_array + idx;
	    fftw_complex*cur2 = src2_array + idx;
	    fftw_complex*curd = dst_array  + idx;

	      // Taking the complex conjugate here defines the
	      // corresponding image as the reference image.
	    cur1[0][1] = -cur1[0][1];

	    fftw_complex res;
	    res[0] = (cur1[0][0] * cur2[0][0]) - (cur1[0][1] * cur2[0][1]);
	    res[1] = (cur1[0][0] * cur2[0][1]) + (cur1[0][1] * cur2[0][0]);

	    double mag = sqrt( pow(res[0],2.0) + pow(res[1],2.0) );
	    curd[0][0] = res[0] / mag;
	    curd[0][1] = res[1] / mag;
      }


      fftw_execute(pland);

      double*res_buf = new double[dst_axes[0]];

	// Convert the result image from complex to double by dropping
	// the now degenerate imaginary part. While we are at it, look
	// for the maximum value. This will be our correlation result.
      vector<long> addr = DataArray::zero_addr(dst_axes.size());
      fftw_complex*ptr = dst_array;
      vector<long> max_ptr = addr;
      double max_val = dst_array[0][0];
      do {
	    for (long idx = 0 ; idx < dst_axes[0] ; idx += 1) {
		  res_buf[idx] = ptr[idx][0];
		  if (res_buf[idx] > max_val) {
			max_val = res_buf[idx];
			max_ptr = addr;
			max_ptr[0] = idx;
		  }
	    }
	    dst->set_line(addr, dst_axes[0], res_buf);
	    ptr += src1_axes[0];
      } while (DataArray::incr(addr, dst_axes, 1));

      delete[]res_buf;

      fftw_destroy_plan(plan1);
      fftw_destroy_plan(plan2);
      fftw_destroy_plan(pland);

      fftw_free(src1_array);
      fftw_free(src2_array);
      fftw_free(dst_array);

	// The resulting offset position assumes the image wraps, and
	// will give a positive result. But we would rather return a
	// small negative value.
      for (size_t idx = 0 ; idx < max_ptr.size() ; idx += 1) {
	    if (max_ptr[idx] > src1_axes[idx]/2)
		  max_ptr[idx] -= src1_axes[idx];
      }

      Tcl_Obj*addr_obj = listobj_from_vector_(max_ptr);
      Tcl_SetObjResult(tcl_engine_, addr_obj);

      return TCL_OK;
}
