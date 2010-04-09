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

int FitsbenchMain::ftcl_fft2d_thunk_(ClientData raw, Tcl_Interp*interp,
				       int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      qassert(eng->tcl_engine_ == interp);
      return eng->ftcl_fft2d_(objc, objv);
}

int FitsbenchMain::ftcl_phase_corr_thunk_(ClientData raw, Tcl_Interp*interp,
				       int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      qassert(eng->tcl_engine_ == interp);
      return eng->ftcl_phase_corr_(objc, objv);
}

template<class src_t> static void do_get_complex_array(const std::vector<long>&axes, fftw_complex*dst, DataArray*src)
{
      src_t*src_buf = new src_t[axes[0]];

      vector<long> src_ptr = DataArray::zero_addr(axes.size());
      do {
	    int rc = src->get_line(src_ptr, axes[0], src_buf);
	    qassert(rc >= 0);
	    for (long idx = 0 ; idx < axes[0] ; idx += 1) {
		  dst[0][0] = src_buf[idx];
		  dst[0][1] = 0.0;
		  dst += 1;
	    }

      } while (DataArray::incr(src_ptr, axes, 1));

      delete[]src_buf;
}

static void get_complex_array(const std::vector<long>& axes, fftw_complex*dst, DataArray*src)
{
      switch (src->get_type()) {
	  case DataArray::DT_UINT8:
	    do_get_complex_array<uint8_t>(axes, dst, src);
	    break;
	  case DataArray::DT_INT8:
	    do_get_complex_array<int8_t>(axes, dst, src);
	    break;
	  case DataArray::DT_UINT16:
	    do_get_complex_array<uint16_t>(axes, dst, src);
	    break;
	  case DataArray::DT_INT16:
	    do_get_complex_array<int16_t>(axes, dst, src);
	    break;
	  default: {
		qassert(0);
		break;
	  }
      }
}

int FitsbenchMain::ftcl_fft2d_(int objc, Tcl_Obj*const objv[])
{
      qassert(objc == 4);

      const char*dst_name = Tcl_GetString(objv[1]);
      if (dst_name == 0)
	    return TCL_ERROR;

      const char*src_name = Tcl_GetString(objv[2]);
      if (src_name == 0)
	    return TCL_ERROR;

      FitsbenchItem* dst_item = item_from_name_(dst_name);
      qassert(dst_item);

      FitsbenchItem* src_item = item_from_name_(src_name);
      qassert(src_item);

      DataArray*dst = dynamic_cast<DataArray*>(dst_item);
      qassert(dst);

      DataArray*src = dynamic_cast<DataArray*>(src_item);
      qassert(src);

      vector<long> addr = vector_from_listobj_(objv[3]);

      vector<long> dst_axes = dst->get_axes();
      vector<long> src_axes = src->get_axes();

	// Make sure the source array has enough axes, and matches the
	// dimensionality of the address.
      qassert(src_axes.size() >= 2);
      qassert(src_axes.size() == addr.size());

	// Make sure the source array has enough width/height to
	// supply data for the result.
      qassert(addr[0] + src_axes[0] <= dst_axes[0]);
      qassert(addr[1] + src_axes[1] <= dst_axes[1]);

	// Everything checks out, so start the actual processing.
      fftw_complex*array = (fftw_complex*)fftw_malloc(dst_axes[0]*dst_axes[1]*sizeof(fftw_complex));
      assert(array);

      fftw_plan plan = fftw_plan_dft_2d(dst_axes[1], dst_axes[0], array, array,
					FFTW_FORWARD, FFTW_ESTIMATE);

      uint8_t*src_buf = new uint8_t[dst_axes[0]];
      vector<long> src_ptr = addr;
      for (int ydx = 0 ; ydx < dst_axes[1] ; ydx += 1, src_ptr[1] += 1) {
	    int rc = src->get_line(src_ptr, dst_axes[1], src_buf);
	    qassert(rc >= 0);
	    fftw_complex*dst_buf = array + ydx*dst_axes[0];
	    for (int xdx = 0 ; xdx < dst_axes[0] ; xdx += 1) {
		  dst_buf[0][0] = src_buf[xdx];
		  dst_buf[0][1] = 0.0;
		  dst_buf += 1;
	    }
      }

      fftw_execute(plan);

      vector<long> dst_ptr (2);
      dst_ptr[0] = 0;
      dst_ptr[1] = 0;
      for (int ydx = 0 ; ydx < dst_axes[1] ; ydx += 1) {
	    complex<double>*dst_buf = reinterpret_cast<complex<double>*>(array + ydx*dst_axes[0]);
	    dst->set_line(dst_ptr, dst_axes[1], dst_buf);
      }

      fftw_destroy_plan(plan);
      fftw_free(array);

      return TCL_OK;
}

int FitsbenchMain::ftcl_phase_corr_(int objc, Tcl_Obj*const objv[])
{
      assert(objc == 4);

      const char*dst_name = Tcl_GetString(objv[1]);
      if (dst_name == 0)
	    return TCL_ERROR;

      const char*src1_name = Tcl_GetString(objv[2]);
      if (src1_name == 0)
	    return TCL_ERROR;

      const char*src2_name = Tcl_GetString(objv[3]);
      if (src2_name == 0)
	    return TCL_ERROR;

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

      if (src1_axes != src2_axes) {
	    Tcl_AppendResult(tcl_engine_, "Axes of sources images must match.", 0);
	    return TCL_ERROR;
      }

      if (dst && dst->get_axes() != src1_axes) {
	    Tcl_AppendResult(tcl_engine_, "Axes of destination image must match source images.", 0);
	    return TCL_ERROR;
      }

	// If the destination image exists, make sure it is a DataArray.
      qassert(dst || !dst_item);

      if (! dst) {
	    QString dst_disp = QString("phase_correlate(%1, %2)") .arg(src1_name) .arg(src2_name);
	    ScratchImage*item = new ScratchImage(dst_disp);
	    ui.bench_tree->addTopLevelItem(item);
	    set_bench_script_name_(item, dst_name);

	    item->reconfig(src1_axes, DataArray::DT_DOUBLE);

	    dst_item = item;
	    dst = item;
      }

      qassert(src1_axes == src2_axes);
      qassert(src1_axes == dst->get_axes());

      size_t pixel_count = DataArray::get_pixel_count(src1_axes);

      fftw_complex*src1_array = (fftw_complex*)fftw_malloc(pixel_count * sizeof(fftw_complex));
      fftw_complex*src2_array = (fftw_complex*)fftw_malloc(pixel_count * sizeof(fftw_complex));
      fftw_complex*dst_array  = (fftw_complex*)fftw_malloc(pixel_count * sizeof(fftw_complex));

      qassert(src1_array);
      qassert(src2_array);
      qassert(dst_array);

      fftw_plan plan1 = fftw_plan_dft_1d(pixel_count, src1_array, src1_array,
					 FFTW_FORWARD, FFTW_ESTIMATE);
      fftw_plan plan2 = fftw_plan_dft_1d(pixel_count, src2_array, src2_array,
					 FFTW_FORWARD, FFTW_ESTIMATE);
      fftw_plan pland = fftw_plan_dft_1d(pixel_count, dst_array, dst_array,
					 FFTW_BACKWARD, FFTW_ESTIMATE);

      get_complex_array(src1_axes, src1_array, src1);
      get_complex_array(src2_axes, src2_array, src2);

      fftw_execute(plan1);
      fftw_execute(plan2);

      for (size_t idx = 0 ; idx < pixel_count ; idx += 1) {
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

      double*res_buf = new double[src1_axes[0]];

	// Convert the result image from complex to double by dropping
	// the now degenerate imaginary part. While we are at it, look
	// for the maximum value. This will be our correlation result.
      vector<long> addr = DataArray::zero_addr(src1_axes.size());
      fftw_complex*ptr = dst_array;
      vector<long> max_ptr = addr;
      double max_val = dst_array[0][0];
      do {
	    for (long idx = 0 ; idx < src1_axes[0] ; idx += 1) {
		  res_buf[idx] = ptr[idx][0];
		  if (res_buf[idx] > max_val) {
			max_val = res_buf[idx];
			max_ptr = addr;
			max_ptr[0] = idx;
		  }
	    }
	    dst->set_line(addr, src1_axes[0], res_buf);
	    ptr += src1_axes[0];
      } while (DataArray::incr(addr, src1_axes, 1));

      delete[]res_buf;

      fftw_destroy_plan(plan1);
      fftw_destroy_plan(plan2);
      fftw_destroy_plan(pland);

      fftw_free(src1_array);
      fftw_free(src2_array);
      fftw_free(dst_array);

      Tcl_Obj*addr_obj = listobj_from_vector_(max_ptr);
      Tcl_SetObjResult(tcl_engine_, addr_obj);

      return TCL_OK;
}
