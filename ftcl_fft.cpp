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

/*
 * Convert the input array to a complex array. The source may have any
 * dimensionality, this will create an array with matching
 * dimensionality. The exception is the interpolation that is
 * done for undefined pixels. This is ONLY 2D.
 */
template<class src_t> static void do_get_complex_array(const std::vector<long>&axes, fftw_complex*dst, DataArray*src, const std::vector<long>&src_pnt)
{
      vector<src_t>   src_buf   (axes[0]);
      vector<src_t>   src_up    (axes[0]);
      vector<src_t>   src_down  (axes[0]);
      vector<uint8_t> alpha_buf (axes[0]);
      vector<uint8_t> alpha_up  (axes[0]);
      vector<uint8_t> alpha_down(axes[0]);

      size_t count = DataArray::get_pixel_count(axes);
      vector<long> src_ptr = src_pnt;
      vector<long> src_ref = src->get_axes();

      int has_alpha = 0;
      int rc = src->get_line(src_ptr, axes[0], &src_buf[0],
			     has_alpha, &alpha_buf[0]);
      DataArray::incr(src_ptr, src_ref, 1);
      for (size_t count_idx = 0 ; count_idx < count ; count_idx += axes[0]) {

	    if (count_idx+1 < count) {
		  rc = src->get_line(src_ptr, axes[0], &src_down[0],
				     has_alpha, &alpha_down[0]);
	    }
	    qassert(rc >= 0);
	    for (long idx = 0 ; idx < axes[0] ; idx += 1) {
		  if (alpha_buf[idx]) {
			dst[0][0] = src_buf[idx];

		  } else {
			  // Pixel value is undefined. Linear
			  // interpolate from up, down, left and right pixels.
			long idx_left = idx-1;
			long idx_right = idx+1;
			double weight = 0.0;
			dst[0][0] = 0.0;
			if (idx_left >= 0 && alpha_buf[idx_left]) {
			      dst[0][0] += src_buf[idx_left];
			      weight += 1.0;
			}
			if (idx_right < axes[0] && alpha_buf[idx_left]) {
			      dst[0][0] += src_buf[idx_right];
			      weight += 1.0;
			}
			if (alpha_up[idx]) {
			      dst[0][0] += src_up[idx];
			      weight += 1.0;
			}
			if (alpha_down[idx]) {
			      dst[0][0] += src_down[idx];
			      weight += 1.0;
			}
			if (weight > 0) {
			      dst[0][0] /= weight;
			}

		  }
		  dst[0][1] = 0.0;
		  dst += 1;
	    }
	    DataArray::incr(src_ptr, src_ref, 1);
	    alpha_up = alpha_buf;
	    alpha_buf= alpha_down;
	    src_up = src_buf;
	    src_buf = src_down;
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
			     "<dst> {axes} <src1> {point} <src2> {point}", 0);
	    return TCL_ERROR;
      }

      const char*dst_name = Tcl_GetString(objv[1]);
      if (dst_name == 0)
	    return TCL_ERROR;

      vector<long> dst_axes;
      vector<long> src1_pnt;
      vector<long> src2_pnt;
      bool delete_dst_when_done = false;

	// Detect an axes list argument that defines the axes of the
	// destination array.
      dst_axes = vector_from_listobj_(objv[2]);

      const char*src1_name = Tcl_GetString(objv[3]);
      if (src1_name == 0)
	    return TCL_ERROR;

	// Detect an optional source point
      const char*tmp = Tcl_GetString(objv[4]);
      if (strcmp(tmp, "-") != 0) 
	    src1_pnt = vector_from_listobj_(objv[4]);

      const char*src2_name = Tcl_GetString(objv[5]);
      if (src2_name == 0)
	    return TCL_ERROR;

	// Detect an optional source point
      tmp = Tcl_GetString(objv[6]);
      if (strcmp(tmp, "-") != 0)
	    src2_pnt = vector_from_listobj_(objv[6]);

      delete_dst_when_done = strcmp(dst_name,"-") == 0? true : false;

      if (! delete_dst_when_done) {
	    FitsbenchItem*dst_item = item_from_name_(dst_name);
	    if (dst_item != 0) {
		  Tcl_AppendResult(tcl_engine_, "Image", dst_name, " already exists", 0);
		  return TCL_ERROR;
	    }
      }

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

      if (dst_axes.size()==0) {
	    Tcl_AppendResult(tcl_engine_, "Destination dimensions unspecified.", 0);
	    return TCL_ERROR;
      }

	// By now the dst_axes must have been figured out.
      qassert(dst_axes.size() != 0);

	// Create the destination item as a scratch image. Note that
	// we only create the destination image if it is not named "-".
      DataArray*dst = 0;

      QString dst_disp = QString("phase_correlate(%1, %2)") .arg(src1_name) .arg(src2_name);
      ScratchImage*dst_scr = new ScratchImage(dst_disp);
      dst_scr->reconfig(dst_axes, DataArray::DT_DOUBLE);

      if (!delete_dst_when_done) {
	    ui.bench_tree->addTopLevelItem(dst_scr);
	    set_bench_script_name_(dst_scr, dst_name);
      }

      dst = dst_scr;


	// If the source point is not otherwise specified, use the
	// upper left corner.
      if (src1_pnt.size() == 0)
	    src1_pnt = DataArray::zero_addr(src1_axes.size());
      if (src2_pnt.size() == 0)
	    src2_pnt = DataArray::zero_addr(src2_axes.size());

      if (dst_axes.size() > src1_axes.size()) {
	    Tcl_AppendResult(tcl_engine_, "Source array ", src1_name,
			     " has too few axes.", 0);
	    return TCL_ERROR;
      }

	// Check that all the arguments to the command make sense, and
	// generate error messages if needed.

      if (dst_axes.size() > src2_axes.size()) {
	    Tcl_AppendResult(tcl_engine_, "Source array ", src2_name,
			     " has too few axes.", 0);
	    return TCL_ERROR;
      }

      if (src1_pnt.size() != src1_axes.size()) {
	    Tcl_AppendResult(tcl_engine_, "Start point for ", src1_name,
			     " has wrong number of dimensions.", 0);
	    return TCL_ERROR;
      }

      if (src2_pnt.size() != src2_axes.size()) {
	    Tcl_AppendResult(tcl_engine_, "Start point for ", src2_name,
			     " has wrong number of dimensions.", 0);
	    return TCL_ERROR;
      }

      for (size_t idx = 0 ; idx < dst_axes.size() ; idx += 1) {
	    if (src1_pnt[idx] + dst_axes[idx] > src1_axes[idx]) {
		  Tcl_AppendResult(tcl_engine_, "Destination array does not fit "
				   "into source ",  src1_name, ".", 0);
		  return TCL_ERROR;
	    }
	    if (src2_pnt[idx] + dst_axes[idx] > src2_axes[idx]) {
		  Tcl_AppendResult(tcl_engine_, "Destination array does not fit "
				   "into source ",  src2_name, ".", 0);
		  return TCL_ERROR;
	    }
      }

	// The pixel_count is the size of the destination array. This
	// may be smaller then the source array, so be careful.
      size_t dst_pixel_count = DataArray::get_pixel_count(dst_axes);

	// We will use these arrays to hold various FFT intermediate
	// results.
      fftw_complex*src1_array = (fftw_complex*)fftw_malloc(dst_pixel_count * sizeof(fftw_complex));
      fftw_complex*src2_array = (fftw_complex*)fftw_malloc(dst_pixel_count * sizeof(fftw_complex));

      qassert(src1_array);
      qassert(src2_array);

	// Plan all our forward and backward FF transforms.
      fftw_plan plan1 = fftw_plan_dft_1d(dst_pixel_count, src1_array, src1_array,
					 FFTW_FORWARD, FFTW_ESTIMATE);
      fftw_plan plan2 = fftw_plan_dft_1d(dst_pixel_count, src2_array, src2_array,
					 FFTW_FORWARD, FFTW_ESTIMATE);
      fftw_plan pland = fftw_plan_dft_1d(dst_pixel_count, src1_array, src1_array,
					 FFTW_BACKWARD, FFTW_ESTIMATE);

	// Get and FFT the source arrays...
      get_complex_array(dst_axes, src1_array, src1, src1_pnt);
      fftw_execute(plan1);

      get_complex_array(dst_axes, src2_array, src2, src2_pnt);
      fftw_execute(plan2);

	// Conjugate the source arrays into the src1 array.
      for (size_t idx = 0 ; idx < dst_pixel_count ; idx += 1) {
	    fftw_complex*cur1 = src1_array + idx;
	    fftw_complex*cur2 = src2_array + idx;

	      // Taking the complex conjugate here defines the
	      // corresponding image as the reference image.
	    cur1[0][1] = -cur1[0][1];

	    fftw_complex res;
	    res[0] = (cur1[0][0] * cur2[0][0]) - (cur1[0][1] * cur2[0][1]);
	    res[1] = (cur1[0][0] * cur2[0][1]) + (cur1[0][1] * cur2[0][0]);

	    double mag = sqrt( pow(res[0],2.0) + pow(res[1],2.0) );
	    cur1[0][0] = res[0] / mag;
	    cur1[0][1] = res[1] / mag;
      }

      fftw_execute(pland);

      fftw_destroy_plan(plan1);
      fftw_destroy_plan(plan2);
      fftw_free(src2_array);

      double*res_buf = new double[dst_axes[0]];

	// Convert the result image from complex to double by dropping
	// the now degenerate imaginary part. While we are at it, look
	// for the maximum and minimum values. We will use those
	// values later to normalize and select correlation results.
      vector<long> addr = DataArray::zero_addr(dst_axes.size());
      fftw_complex*ptr = src1_array;
      double max_val = src1_array[0][0];
      double min_val = src1_array[0][0];
      do {
	    for (long idx = 0 ; idx < dst_axes[0] ; idx += 1) {
		  res_buf[idx] = ptr[idx][0];
		  if (res_buf[idx] < min_val)
			min_val = res_buf[idx];
		  if (res_buf[idx] > max_val)
			max_val = res_buf[idx];
	    }
	    dst->set_line(addr, dst_axes[0], res_buf);
	    ptr += src1_axes[0];
      } while (DataArray::incr(addr, dst_axes, 1));

	// Find the maximum by finding the barycenter of the
	// array. Bias the array so that the min value is zero, and
	// threshold at (max-min)/sqrt(2) so that only "interesting"
	// points go into locating the peak.

	// Use the barycenter instead of the simple maximum so that we
	// get sub-pixel accuracy. (Am I fooling myself?) Note that
	// the expectation is that the barycenter will probably be
	// near the origin, and the phase correlation generates a
	// circular result, so shift coordinates half way to the far
	// side to the negative part of the axis.

      double test_thresh = (max_val - min_val) * 0.707;
      vector<double> moment (dst_axes.size());
      double mass = 0.0;
      addr = DataArray::zero_addr(dst_axes.size());
      do {
	    int has_alpha = 0;
	    dst->get_line(addr, dst_axes[0], res_buf, has_alpha);
	    for (long idx = 0 ; idx < dst_axes[0] ; idx += 1) {
		  res_buf[idx] -= min_val;
		  if (res_buf[idx] <= test_thresh)
			continue;

		  double arm = idx;
		  if (arm >= dst_axes[0]/2)
			arm -= dst_axes[0];
		  mass += res_buf[idx];
		  moment[0] += arm * res_buf[idx];
		  for (size_t coord = 1 ; coord < moment.size() ; coord += 1) {
			arm = addr[coord];
			if (arm >= dst_axes[coord]/2)
			      arm -= dst_axes[coord];
			moment[coord] += arm * res_buf[idx];
		  }
	    }
      } while (DataArray::incr(addr, dst_axes, 1));

      if (delete_dst_when_done)
	    delete dst;

      vector<double> max_ptr (addr.size());
      for (size_t coord = 0 ; coord < moment.size() ; coord += 1)
	    max_ptr[coord] = moment[coord] / mass;

      delete[]res_buf;

      fftw_destroy_plan(pland);
      fftw_free(src1_array);

      Tcl_Obj*addr_obj = listobj_from_vector_(max_ptr);
      Tcl_SetObjResult(tcl_engine_, addr_obj);

      return TCL_OK;
}
