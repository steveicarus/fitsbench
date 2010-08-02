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
# include  <tiffio.h>
# include  "qassert.h"

using namespace std;
static int import_tiff(WorkFolder*work, const QString&name, const char*path);
static int import_pnm(WorkFolder*work, const QString&name, const char*path);


int FitsbenchMain::ftcl_import_thunk_(ClientData raw, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      qassert(eng->tcl_engine_ == interp);
      return eng->ftcl_import_(objc, objv);
}

/*
 * import <work/name> <path>
 */

int FitsbenchMain::ftcl_import_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 3) {
	    Tcl_AppendResult(tcl_engine_, "usage: import <work/name> <path>", 0);
	    return TCL_ERROR;
      }

      QString name;
      WorkFolder*work = workfolder_from_name_(objv[1], name);
      if (work == 0) {
	    Tcl_AppendResult(tcl_engine_, "Unable to find work folder", 0);
	    return TCL_ERROR;
      }

      const char*path = Tcl_GetString(objv[2]);
      const char*suff = strrchr(path, '.');

      if (suff == 0) {
	    Tcl_AppendResult(tcl_engine_, "I don't know what ", path, " is.", 0);
	    return TCL_ERROR;
      }

      if (strcasecmp(suff, ".tif") == 0) {
	    return import_tiff(work, name, path);
      } else if (strcasecmp(suff, ".tiff") == 0) {
	    return import_tiff(work, name, path);
      } else if (strcasecmp(suff, ".pgm") == 0) {
	    return import_pnm(work, name, path);
      } else if (strcasecmp(suff, ".ppm") == 0) {
	    return import_pnm(work, name, path);
      } else {
	    Tcl_AppendResult(tcl_engine_, "I don't know what ", path, " is.", 0);
	    return TCL_ERROR;
      }
}


template <class data_t> static void import_tiff_gray(WorkFolder::Image*img, TIFF*fd, data_t*buf)
{
      vector<long> axes = img->get_axes();
      qassert(axes.size() == 2);

      vector<long> dst (2);
      dst[0] = 0;

      for (dst[1] = 0 ; dst[1] < axes[1] ; dst[1] += 1) {
	    TIFFReadScanline(fd, buf, dst[1], 0);
	    img->set_line(dst, axes[0], buf);
      }
}

template <class data_t> static void import_tiff_color(WorkFolder::Image*img, TIFF*fd, data_t*buf)
{
      vector<long> axes = img->get_axes();
      qassert(axes.size() == 3);

      vector<long> dst (3);
      dst[0] = 0;

      vector<data_t*> planes ( axes[2] );
      for (int clr = 0 ; clr < axes[2] ; clr += 1)
	    planes[clr] = new data_t [axes[0]];

      for (dst[1] = 0 ; dst[1] < axes[1] ; dst[1] += 1) {
	    TIFFReadScanline(fd, buf, dst[1], 0);
	    for (int xdx = 0 ; xdx < axes[0] ; xdx += 1) {
		  for (int clr = 0 ; clr < axes[2] ; clr += 1)
			planes[clr][xdx] = buf[xdx*axes[2] + clr];
	    }
	    for (int clr = 0 ; clr < axes[2] ; clr += 1) {
		  dst[2] = clr;
		  img->set_line(dst, axes[0], planes[clr]);
	    }
      }

      for (int clr = 0 ; clr < axes[2] ; clr += 1)
	    delete[] planes[clr];
}

static int import_tiff(WorkFolder*work, const QString&name, const char*path)
{
      TIFF*fd = TIFFOpen(path, "r");

      uint16 planes;
      TIFFGetFieldDefaulted(fd, TIFFTAG_SAMPLESPERPIXEL, &planes);

      uint32 wid, hei;
      TIFFGetFieldDefaulted(fd, TIFFTAG_IMAGEWIDTH,  &wid);
      TIFFGetFieldDefaulted(fd, TIFFTAG_IMAGELENGTH, &hei);

      vector<long> axes ( planes > 1? 3 : 2 );
      axes[0] = wid;
      axes[1] = hei;
      if (planes > 1)
	    axes[2] = planes;

      uint16 bits_per_sample;
      TIFFGetFieldDefaulted(fd, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);

      DataArray::type_t type;
      if (bits_per_sample <= 8)
	    type = DataArray::DT_UINT8;
      else if (bits_per_sample <= 16)
	    type = DataArray::DT_UINT16;
      else if (bits_per_sample <= 32)
	    type = DataArray::DT_UINT32;
      else
	    type = DataArray::DT_UINT64;

      WorkFolder::Image*img = new WorkFolder::Image(work, name);

      img->reconfig(axes, type);

      switch (type) {
	  case DataArray::DT_UINT8: {
		uint8_t*buf = new uint8_t [axes[0] * planes];
		if (planes==1)
		      import_tiff_gray(img, fd, buf);
		else
		      import_tiff_color(img, fd, buf);
		delete[]buf;
		break;
	  }
	  case DataArray::DT_UINT16: {
		uint16_t*buf = new uint16_t [axes[0] * planes];
		if (planes==1)
		      import_tiff_gray(img, fd, buf);
		else
		      import_tiff_color(img, fd, buf);
		delete[]buf;
		break;
	  }
	  default:
	    break;
      }

      TIFFClose(fd);
      return TCL_OK;
}

static int import_pnm(WorkFolder*work, const QString&name, const char*path)
{
      return TCL_OK;
}
