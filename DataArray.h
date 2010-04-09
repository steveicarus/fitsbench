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
# include  <fitsio.h>

/*
 * The DataArray is an abstract class that represents objects that can
 * be interpreted as an N-dimensional data array.
 */
class DataArray {

    public:
	// These are image pixel types that the DataArray
	// supports. Most have matching cfitsio definitions and a way
	// to represent them in a FITS IMAGE HDU. There are a few that
	// don't match anything, they are only used internally.
      enum type_t {
	      //         CFITSIO          Equivilent FITS declaration
	    DT_VOID    = 0,
	    DT_INT8    = SBYTE_IMG,   //  BITPIX=  8, BZERO = -128
	    DT_UINT8   = BYTE_IMG,    //  BITPIX=  8, BZERO = 0
	    DT_INT16   = SHORT_IMG,   //  BITPIX= 16, BZERO = 0
	    DT_UINT16  = USHORT_IMG,  //  BITPIX= 16, BZERO = 32768
	    DT_INT32   = LONG_IMG,    //  BITPIX= 32, BZERO = 0
	    DT_UINT32  = ULONG_IMG,   //  BITPIX= 32, BZERO = 2147483648
	    DT_INT64   = LONGLONG_IMG,//  BITPIX= 64, BZERO = 0
	    DT_UINT64  = 65,          //  BITPIX= 64, BZERO = <very large>
	    DT_FLOAT32 = FLOAT_IMG,   //  BITPIX=-32
	    DT_FLOAT64 = DOUBLE_IMG,  //  BITPIX=-64
	    DT_DOUBLE  = -65,         //  (Native double)
	    DT_COMPLEX = -66          //  (Native double complex)
      };

      static type_t type_from_string(const std::string&str);

    public:
      DataArray() { }
      virtual ~DataArray() =0;

      virtual std::vector<long> get_axes(void) const;
      virtual type_t get_type(void) const { return DT_VOID; }

      template <class T>
      int set_line(const std::vector<long>&addr, long wid, const T*data);

      template <class T>
      int get_line(const std::vector<long>&addr, long wid, T*data);

      virtual int set_line_raw(const std::vector<long>&addr, long wid,
			       type_t type, const void*data);

      virtual int get_line_raw(const std::vector<long>&addr, long wid,
			       type_t type, void*data);

    public:
	// These are some convenient N-dimensional address
	// manipulation functions.

      static size_t get_pixel_count(const std::vector<long>&axes);

      static std::vector<long> zero_addr(size_t axes);

	// Increment the addr at the given axis within the ref axes
	// dimensions. If the axis wraps, the next axis is
	// incremented, and so on. If the address overflows, it wraps
	// to pixel 0 and this function returns false.
      static bool incr(std::vector<long>&addr, const std::vector<long>&ref, size_t axis);
      static std::vector<long> add(const std::vector<long>&a, const std::vector<long>&b);
};

template <> inline int DataArray::set_line<int8_t>(const std::vector<long>&addr, long wid, const int8_t*data)
{ return set_line_raw(addr, wid, DT_INT8, data); }
template <> inline int DataArray::set_line<int16_t>(const std::vector<long>&addr, long wid, const int16_t*data)
{ return set_line_raw(addr, wid, DT_INT16, data); }
template <> inline int DataArray::set_line<int32_t>(const std::vector<long>&addr, long wid, const int32_t*data)
{ return set_line_raw(addr, wid, DT_INT32, data); }
template <> inline int DataArray::set_line<int64_t>(const std::vector<long>&addr, long wid, const int64_t*data)
{ return set_line_raw(addr, wid, DT_INT64, data); }

template <> inline int DataArray::set_line<uint8_t>(const std::vector<long>&addr, long wid, const uint8_t*data)
{ return set_line_raw(addr, wid, DT_UINT8, data); }
template <> inline int DataArray::set_line<uint16_t>(const std::vector<long>&addr, long wid, const uint16_t*data)
{ return set_line_raw(addr, wid, DT_UINT16, data); }
template <> inline int DataArray::set_line<uint32_t>(const std::vector<long>&addr, long wid, const uint32_t*data)
{ return set_line_raw(addr, wid, DT_UINT32, data); }
template <> inline int DataArray::set_line<uint64_t>(const std::vector<long>&addr, long wid, const uint64_t*data)
{ return set_line_raw(addr, wid, DT_UINT64, data); }

template <> inline int DataArray::set_line<double>(const std::vector<long>&addr, long wid, const double*data)
{ return set_line_raw(addr, wid, DT_DOUBLE, data); }

template <> inline int DataArray::set_line< std::complex<double> >(const std::vector<long>&addr, long wid, const std::complex<double>*data)
{ return set_line_raw(addr, wid, DT_COMPLEX, data); }

template <> inline int DataArray::get_line<int8_t>(const std::vector<long>&addr, long wid, int8_t*data)
{ return get_line_raw(addr, wid, DT_INT8, data); }
template <> inline int DataArray::get_line<int16_t>(const std::vector<long>&addr, long wid, int16_t*data)
{ return get_line_raw(addr, wid, DT_INT16, data); }
template <> inline int DataArray::get_line<int32_t>(const std::vector<long>&addr, long wid, int32_t*data)
{ return get_line_raw(addr, wid, DT_INT32, data); }
template <> inline int DataArray::get_line<int64_t>(const std::vector<long>&addr, long wid, int64_t*data)
{ return get_line_raw(addr, wid, DT_INT64, data); }

template <> inline int DataArray::get_line<uint8_t>(const std::vector<long>&addr, long wid, uint8_t*data)
{ return get_line_raw(addr, wid, DT_UINT8, data); }
template <> inline int DataArray::get_line<uint16_t>(const std::vector<long>&addr, long wid, uint16_t*data)
{ return get_line_raw(addr, wid, DT_UINT16, data); }
template <> inline int DataArray::get_line<uint32_t>(const std::vector<long>&addr, long wid, uint32_t*data)
{ return get_line_raw(addr, wid, DT_UINT32, data); }
template <> inline int DataArray::get_line<uint64_t>(const std::vector<long>&addr, long wid, uint64_t*data)
{ return get_line_raw(addr, wid, DT_UINT64, data); }

template <> inline int DataArray::get_line<double>(const std::vector<long>&addr, long wid, double*data)
{ return get_line_raw(addr, wid, DT_DOUBLE, data); }

#endif
