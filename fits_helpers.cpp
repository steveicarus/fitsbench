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

# include  "fits_helpers.h"
# include  <QImage>
# include  <QMessageBox>
# include  <QString>
# include  <vector>
# include  "qassert.h"

using namespace std;

void show_fits_error_stack(const QString&title)
{
      char msg[FLEN_ERRMSG];
      while (fits_read_errmsg(msg)) {
	    QMessageBox::warning(0, title, msg);
      }
}

template <class T> static void do_render_fits(fitsfile*fd, int datatype,
					      int naxis, const vector<long>&naxes,
					      int ridx, int gidx, int bidx,
					      T*rrow, T*grow, T*brow,
					      QImage&image, int*status)
{
      vector<long> fpixel (naxis);
      for (int idx = 0 ; idx < naxis ; idx += 1)
	    fpixel[idx] = 1;

      vector<char> rnull (naxes[0]);
      vector<char> gnull (naxes[0]);
      vector<char> bnull (naxes[0]);

      T maxval = 0;

      for (int ydx = 0 ; ydx < naxes[1] ; ydx += 1) {
	    fpixel[1] = ydx+1;
	    fpixel[2] = ridx;
	    int anynul = 0;

	    fits_read_pixnull(fd, datatype, &fpixel[0], naxes[0],
			      rrow, &rnull[0], &anynul, status);
	    for (int xdx = 0 ; xdx < naxes[0] ; xdx += 1) {
		  if (rrow[xdx] > maxval && !rnull[xdx])
			maxval = rrow[xdx];
	    }
      }

      if (gidx != ridx) for (int ydx = 0 ; ydx < naxes[1] ; ydx += 1) {
	    fpixel[1] = ydx+1;
	    fpixel[2] = gidx;
	    int anynul = 0;

	    fits_read_pixnull(fd, datatype, &fpixel[0], naxes[0],
			      grow, &gnull[0], &anynul, status);
	    for (int xdx = 0 ; xdx < naxes[0] ; xdx += 1) {
		  if (grow[xdx] > maxval && !gnull[xdx])
			maxval = grow[xdx];
	    }
      }

      if (bidx != ridx) for (int ydx = 0 ; ydx < naxes[1] ; ydx += 1) {
	    fpixel[1] = ydx+1;
	    fpixel[2] = bidx;
	    int anynul = 0;

	    fits_read_pixnull(fd, datatype, &fpixel[0], naxes[0],
			      brow, &bnull[0], &anynul, status);
	    for (int xdx = 0 ; xdx < naxes[0] ; xdx += 1) {
		  if (brow[xdx] > maxval && !bnull[xdx])
			maxval = brow[xdx];
	    }
      }

      uint32_t scale = 255;
      if (maxval >= 4)
	    scale = maxval;

      for (int ydx = 0 ; ydx < naxes[1] ; ydx += 1) {
	    fpixel[1] = ydx+1;
	    fpixel[2] = ridx;
	    int anynul = 0;

	    fits_read_pixnull(fd, datatype, &fpixel[0], naxes[0],
			      rrow, &rnull[0], &anynul, status);
	    fpixel[2] = gidx;
	    anynul = 0;
	    fits_read_pixnull(fd, datatype, &fpixel[0], naxes[0],
			      grow, &gnull[0], &anynul, status);
	    fpixel[2] = bidx;
	    anynul = 0;
	    fits_read_pixnull(fd, datatype, &fpixel[0], naxes[0],
			      brow, &bnull[0], &anynul, status);

	    for (int xdx = 0 ; xdx < naxes[0] ; xdx += 1) {
		  int alpha = (rnull[xdx] && gnull[xdx] && bnull[xdx])? 0 : 0xff;
		  uint32_t tmp;
		  if (rnull[xdx] == 0) {
			tmp = rrow[xdx];
			tmp = tmp * 255 / scale;
			if (tmp > 255) tmp = 255;
			rrow[xdx] = tmp;
		  } else {
			rrow[xdx] = 0;
		  }
		  if (gnull[xdx] == 0) {
			tmp = grow[xdx];
			tmp = tmp * 255 / scale;
			if (tmp > 255) tmp = 255;
			grow[xdx] = tmp;
		  } else {
			grow[xdx] = 0;
		  }
		  if (bnull[xdx] == 0) {
			tmp = brow[xdx];
			tmp = tmp * 255 / scale;
			if (tmp > 255) tmp = 255;
			brow[xdx] = tmp;
		  } else {
			brow[xdx] = 0;
		  }

		  image.setPixel(xdx, ydx, qRgba(rrow[xdx], grow[xdx], brow[xdx], alpha));
	    }
      }
}

int render_chdu_into_qimage(fitsfile*fd, int ridx, int gidx, int bidx,
			    QImage&image, int*status)
{
      int bitpix = 0;
      fits_get_img_equivtype(fd, &bitpix, status);

      int naxis = 0;
      fits_get_img_dim(fd, &naxis, status);

      qassert(naxis >= 2);

      vector<long>naxes (naxis);
      fits_get_img_size(fd, naxis, &naxes[0], status);

      qassert(ridx == 1 || naxis >= 3 && ridx <= naxes[2]);
      qassert(gidx == 1 || naxis >= 3 && gidx <= naxes[2]);
      qassert(bidx == 1 || naxis >= 3 && bidx <= naxes[2]);

      image = QImage(naxes[0], naxes[1], QImage::Format_ARGB32);

      if (bitpix == BYTE_IMG) {
	    unsigned char*rrow = new unsigned char [naxes[0]];
	    unsigned char*grow = new unsigned char [naxes[0]];
	    unsigned char*brow = new unsigned char [naxes[0]];
	    do_render_fits(fd, TBYTE, naxis, naxes, ridx, gidx, bidx,
			   rrow, grow, brow, image, status);
	    delete[]rrow;
	    delete[]grow;
	    delete[]brow;

      } else if (bitpix == USHORT_IMG) {
	    unsigned short*rrow = new unsigned short [naxes[0]];
	    unsigned short*grow = new unsigned short [naxes[0]];
	    unsigned short*brow = new unsigned short [naxes[0]];
	    do_render_fits(fd, TUSHORT, naxis, naxes, ridx, gidx, bidx,
			   rrow, grow, brow, image, status);
	    delete[]rrow;
	    delete[]grow;
	    delete[]brow;
      } else {
	    QString text = QString ("I don't know how to render bitpix=%1").arg(bitpix);
	    QMessageBox::warning(0, "FITS render error", text);
      }

      return *status;
}
