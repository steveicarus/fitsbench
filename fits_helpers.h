#ifndef __fits_helpers_H
#define __fits_helpers_H
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

# include  <fitsio.h>
class QImage;
class QString;

extern void show_fits_error_stack(const QString&str);

/*
 * Render the current HDU of a fits file into a QImage. The ridx, gidx
 * and bidx are the indices (FITS conventions...) of the red, green
 * and blue planes. If the image is grayscale (has only 1 plane) then
 * pass 1 to all these values.
 */
extern int render_chdu_into_qimage(fitsfile*fd, int ridx, int gidx, int bidx,
				   QImage&image, int*status);


#endif
