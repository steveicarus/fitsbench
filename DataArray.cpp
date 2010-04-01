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

# include  "DataArray.h"

DataArray::~DataArray()
{
}

std::vector<long> DataArray::get_axes(void) const
{
      return std::vector<long> ();
}

int DataArray::set_line_raw(const std::vector<long>&, long, type_t, const void*)
{
      return -1;
}

int DataArray::get_line_raw(const std::vector<long>&, long, type_t, void*)
{
      return -1;
}

DataArray::type_t DataArray::type_from_string(const std::string&str)
{
      const static struct {
	    const char*name;
	    DataArray::type_t code;
      } type[] = {
	    { "int8",   DT_INT8 },
	    { "int16",  DT_INT16 },
	    { "int32",  DT_INT32 },
	    { "int64",  DT_INT64 },
	    { "uint8",  DT_UINT8 },
	    { "uint16", DT_UINT16 },
	    { "uint32", DT_UINT32 },
	    { "uint64", DT_UINT64 },
	    { "float32",DT_FLOAT32 },
	    { "float64",DT_FLOAT64 },
	    { "double", DT_DOUBLE },
	    { "complex",DT_COMPLEX },
	    { 0, DT_VOID }
      };

      for (int idx = 0 ; type[idx].name != 0 ; idx += 1) {
	    if (str == type[idx].name)
		  return type[idx].code;
      }

      return DT_VOID;
}
