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

# include  "DataTable.h"
# include  <cassert>

using namespace std;

DataTable::type_t DataTable::type_from_string(const string&str)
{
      const static struct {
	    const char*name;
	    DataTable::type_t code;
      } type[] = {
	    { "uint8",  DT_UINT8 },
	    { "int16",  DT_INT16 },
	    { "int32",  DT_INT32 },
	    { "double", DT_DOUBLE },
	    { "string", DT_STRING },
	    { 0, DT_VOID }
      };

      for (int idx = 0 ; type[idx].name != 0 ; idx += 1) {
	    if (str == type[idx].name)
		  return type[idx].code;
      }

      return DT_VOID;
}

DataTable::DataTable()
{
}

DataTable::~DataTable()
{
}
