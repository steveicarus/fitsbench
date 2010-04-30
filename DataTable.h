#ifndef __DataTable_H
#define __DataTable_H
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

# include  <stdint.h>
# include  <string>
# include  <QString>
# include  <fitsio.h>

/*
 * The DataTable is an abstract class that represents objects that can
 * be interpreted as tables with columns and rows.
 */
class DataTable {

    public:
      enum type_t {
	    DT_VOID,
	    DT_CHAR,   // ASCII Character
	    DT_UINT8,
	    DT_INT16,
	    DT_INT32,
	    DT_STRING  // Variable array of ASCII
      };

      struct column_t {
	    column_t(void) : type(DT_VOID), repeat(0), max_elements(0) { }
	    QString heading;
	    DataTable::type_t type;
	    int repeat;
	    size_t max_elements;
      };

      static type_t type_from_string(const std::string&str);

    public:
      DataTable();
      virtual ~DataTable() =0;

      virtual size_t table_cols() =0;
      virtual size_t table_rows() =0;

      virtual column_t table_col_info(size_t col) =0;

      virtual uint8_t table_value_uint8(size_t row, size_t col) =0;
      virtual int16_t table_value_int16(size_t row, size_t col) =0;
      virtual int32_t table_value_int32(size_t row, size_t col) =0;
      virtual QString table_value_string(size_t row, size_t col) =0;
};

#endif
