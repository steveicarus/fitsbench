#ifndef __DataArray_H
#define __DataArray_H
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
# include  <complex>
# include  <vector>
# include  <string>

/*
 * The DataArray is an abstract class that represents objects that can
 * be interpreted as an N-dimensional data array.
 */
class DataArray {

    public:
      enum type_t {
	    DT_VOID = 0,
	    DT_INT8 = 8,    DT_UINT8 = 9,
	    DT_INT16 = 16,  DT_UINT16 = 17,
	    DT_INT32 = 32,  DT_UINT32 = 33,
	    DT_INT64 = 64,  DT_UINT64 = 65,
	    DT_FLOAT32 = -32,
	    DT_FLOAT64 = -64,
	    DT_DOUBLE  = -65,  /* Native double */
	    DT_COMPLEX = -66   /* Native double complex */
      };

      static type_t type_from_string(const std::string&str);

    public:
      DataArray() { }
      virtual ~DataArray() =0;

      virtual std::vector<long> get_axes(void) const;
      virtual type_t get_type(void) const { return DT_VOID; }

      int set_line(const std::vector<long>&addr, long wid, const int8_t*data);
      int set_line(const std::vector<long>&addr, long wid, const int16_t*data);
      int set_line(const std::vector<long>&addr, long wid, const int32_t*data);
      int set_line(const std::vector<long>&addr, long wid, const int64_t*data);

      int set_line(const std::vector<long>&addr, long wid, const uint8_t*data);
      int set_line(const std::vector<long>&addr, long wid, const uint16_t*data);
      int set_line(const std::vector<long>&addr, long wid, const uint32_t*data);
      int set_line(const std::vector<long>&addr, long wid, const uint64_t*data);

      int set_line(const std::vector<long>&addr, long wid, const double*data);
      int set_line(const std::vector<long>&addr, long wid, const std::complex<double>*data);

      virtual int set_line_raw(const std::vector<long>&addr, long wid,
			       type_t type, const void*data);

      int get_line(const std::vector<long>&addr, long wid, int8_t*data);
      int get_line(const std::vector<long>&addr, long wid, int16_t*data);
      int get_line(const std::vector<long>&addr, long wid, int32_t*data);
      int get_line(const std::vector<long>&addr, long wid, int64_t*data);

      int get_line(const std::vector<long>&addr, long wid, uint8_t*data);
      int get_line(const std::vector<long>&addr, long wid, uint16_t*data);
      int get_line(const std::vector<long>&addr, long wid, uint32_t*data);
      int get_line(const std::vector<long>&addr, long wid, uint64_t*data);

      int get_line(const std::vector<long>&addr, long wid, double*data);
      int get_line(const std::vector<long>&addr, long wid, std::complex<double>*data);

      virtual int get_line_raw(const std::vector<long>&addr, long wid,
			       type_t type, void*data);

};

inline int DataArray::set_line(const std::vector<long>&addr, long wid, const int8_t*data)
{ return set_line_raw(addr, wid, DT_INT8, data); }
inline int DataArray::set_line(const std::vector<long>&addr, long wid, const int16_t*data)
{ return set_line_raw(addr, wid, DT_INT16, data); }
inline int DataArray::set_line(const std::vector<long>&addr, long wid, const int32_t*data)
{ return set_line_raw(addr, wid, DT_INT32, data); }
inline int DataArray::set_line(const std::vector<long>&addr, long wid, const int64_t*data)
{ return set_line_raw(addr, wid, DT_INT64, data); }

inline int DataArray::set_line(const std::vector<long>&addr, long wid, const uint8_t*data)
{ return set_line_raw(addr, wid, DT_UINT8, data); }
inline int DataArray::set_line(const std::vector<long>&addr, long wid, const uint16_t*data)
{ return set_line_raw(addr, wid, DT_UINT16, data); }
inline int DataArray::set_line(const std::vector<long>&addr, long wid, const uint32_t*data)
{ return set_line_raw(addr, wid, DT_UINT32, data); }
inline int DataArray::set_line(const std::vector<long>&addr, long wid, const uint64_t*data)
{ return set_line_raw(addr, wid, DT_UINT64, data); }

inline int DataArray::set_line(const std::vector<long>&addr, long wid, const double*data)
{ return set_line_raw(addr, wid, DT_DOUBLE, data); }

inline int DataArray::get_line(const std::vector<long>&addr, long wid, int8_t*data)
{ return get_line_raw(addr, wid, DT_INT8, data); }
inline int DataArray::get_line(const std::vector<long>&addr, long wid, int16_t*data)
{ return get_line_raw(addr, wid, DT_INT16, data); }
inline int DataArray::get_line(const std::vector<long>&addr, long wid, int32_t*data)
{ return get_line_raw(addr, wid, DT_INT32, data); }
inline int DataArray::get_line(const std::vector<long>&addr, long wid, int64_t*data)
{ return get_line_raw(addr, wid, DT_INT64, data); }

inline int DataArray::get_line(const std::vector<long>&addr, long wid, uint8_t*data)
{ return get_line_raw(addr, wid, DT_UINT8, data); }
inline int DataArray::get_line(const std::vector<long>&addr, long wid, uint16_t*data)
{ return get_line_raw(addr, wid, DT_UINT16, data); }
inline int DataArray::get_line(const std::vector<long>&addr, long wid, uint32_t*data)
{ return get_line_raw(addr, wid, DT_UINT32, data); }
inline int DataArray::get_line(const std::vector<long>&addr, long wid, uint64_t*data)
{ return get_line_raw(addr, wid, DT_UINT64, data); }

inline int DataArray::get_line(const std::vector<long>&addr, long wid, double*data)
{ return get_line_raw(addr, wid, DT_DOUBLE, data); }

#endif
