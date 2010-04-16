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

# include  "FitsbenchItem.h"
# include  <QDir>
# include  "qassert.h"

using namespace std;

WorkFolder::WorkFolder(const QString&name, const QDir&path)
: BenchFile(name, path.path())
{
}

WorkFolder::~WorkFolder()
{
}

WorkFolder::Image* WorkFolder::get_image(const QString&name)
{
      Image*&ptr = child_map_[name];

      if (ptr == 0) {
	    ptr = new Image(this, name);
      }

      return ptr;
}

/*
 * Implement the WorkFolder items using FITS files. An image is a
 * fits file with a single HDU that contains the data array.
 */
WorkFolder::Image::Image(WorkFolder*folder, const QString&name)
: FitsbenchItem(folder)
{
      fd_ = 0;
      setDisplayName(name);
}

WorkFolder::Image::~Image()
{
      if (fd_) {
	    int status = 0;
	    fits_close_file(fd_, &status);
      }
}

WorkFolder* WorkFolder::Image::folder()
{
      return dynamic_cast<WorkFolder*> (parent());
}

int WorkFolder::Image::copy_from_array(const DataArray*that)
{
	// Not implemented yet
      return -1;
}
