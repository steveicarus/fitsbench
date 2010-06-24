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
# include  "fits_helpers.h"
# include  <QDir>
# include  <QTableWidget>
# include  "SimpleImageView.h"
# include  "SimpleTableView.h"
# include  <iostream>
# include  "qassert.h"

using namespace std;

WorkFolder::WorkFolder(const QString&name, const QDir&path)
: BenchFile(name, path.path()),
  work_path_(path),
  work_settings_(path.filePath("WorkFolder.ini"),QSettings::IniFormat)
{
	// If there is a persistent script name, then use that.
      QString tmp = work_settings_.value("gui/script_name").toString();
      if (! tmp.isEmpty())
	    setScriptName(tmp);

      QStringList name_filters;
      name_filters << "*.fits";
      work_path_.setNameFilters(name_filters);

      QStringList files = work_path_.entryList();

      for (int idx = 0 ; idx < files.size() ; idx += 1) {
	    QString name = files[idx];
	    QFileInfo item_path (path, name);

	    if (name.endsWith("-i.fits")) {
		  name.chop(7);
		  map_folder_item(name, new Image(this, name, item_path));
	    } else if (name.endsWith("-t.fits")) {
		  name.chop(7);
		  map_folder_item(name, new Table(this, name, item_path));
	    }
      }
}

WorkFolder::~WorkFolder()
{
      work_settings_.setValue("gui/script_name", getScriptName());

	// We do not have to delete the items in the child_map_
	// because the are QTreeWidgetItems that are children of me as
	// a QTreeWidgetItem; Qt as already deleted them for me.
}

WorkFolder::Image* WorkFolder::get_image(const QString&name)
{
      Image*ptr = image_map_[name];

      if (ptr == 0) {
	    ptr = new Image(this, name);
	    map_folder_item(name, ptr);
      }

      return ptr;
}

WorkFolder::Table* WorkFolder::find_table(const QString&name) const
{
      map<QString,Table*>::const_iterator cur = table_map_.find(name);
      if (cur == table_map_.end())
	    return 0;
      else
	    return cur->second;
}

WorkFolder::Image* WorkFolder::find_image(const QString&name) const
{
      map<QString,Image*>::const_iterator cur = image_map_.find(name);
      if (cur == image_map_.end())
	    return 0;
      else
	    return cur->second;
}

FitsbenchItem* WorkFolder::find_item(const QString&name) const
{
      if (FitsbenchItem*cur = find_image(name))
	    return cur;
      if (FitsbenchItem*cur = find_table(name))
	    return cur;

      return 0;
}

std::vector<QString> WorkFolder::image_names() const
{
      vector<QString> res;
      for (map<QString,Image*>::const_iterator cur = image_map_.begin()
		 ; cur != image_map_.end() ; cur ++) {
	    res.push_back(cur->first);
      }

      return res;
}

std::vector<QString> WorkFolder::table_names() const
{
      vector<QString> res;
      for (map<QString,Table*>::const_iterator cur = table_map_.begin()
		 ; cur != table_map_.end() ; cur ++) {
	    res.push_back(cur->first);
      }

      return res;
}

void WorkFolder::map_folder_item(const QString&key, WorkFolder::WorkFits*item)
{
      qassert(item->parent() == this);

      if (Table*cur = dynamic_cast<Table*> (item)) {
	    qassert( table_map_[key] == 0 );
	    table_map_[key] = cur;
	    return;
      }

      if (Image*cur = dynamic_cast<Image*> (item)) {
	    qassert( image_map_[key] == 0 );
	    image_map_[key] = cur;
	    return;
      }

      qinternal_error("Unexpected WorkFits type");
}

WorkFolder::WorkFits::WorkFits(WorkFolder*folder, const QString&name)
: FitsbenchItem(folder)
{
      fd_ = 0;
      setDisplayName(name);
}

WorkFolder::WorkFits::WorkFits(WorkFolder*folder, const QString&name, const QFileInfo&file)
: FitsbenchItem(folder)
{
      qassert(file.exists());
      setDisplayName(name);

      int status = 0;
      fits_open_diskfile(&fd_, file.filePath().toLocal8Bit().constData(),
			 READWRITE, &status);
      if (status != 0) {
	    show_fits_error_stack(QString("Error opening %1").arg(file.filePath()));
      }
}

WorkFolder::WorkFits::~WorkFits()
{
      if (fd_) {
	    int status = 0;
	    fits_close_file(fd_, &status);
      }
}

WorkFolder* WorkFolder::WorkFits::folder()
{
      return dynamic_cast<WorkFolder*> (parent());
}

void WorkFolder::WorkFits::fill_in_info_table(QTableWidget*widget)
{
      if (fd_ == 0)
	    return;

      int status = 0;
      int nkeys = 0;
      int morekeys = 0;
      fits_get_hdrspace(fd_, &nkeys, &morekeys, &status);

      widget->setRowCount(nkeys);

      for (int idx = 0 ; idx < nkeys ; idx += 1) {
	    char key_buf [FLEN_KEYWORD];
	    char val_buf [FLEN_VALUE];
	    char com_buf [FLEN_COMMENT];
	    fits_read_keyn(fd_, idx+1, key_buf, val_buf, com_buf, &status);
	    widget->setItem(idx, 0, new QTableWidgetItem(key_buf));
	    widget->setItem(idx, 1, new QTableWidgetItem(val_buf));
	    widget->setItem(idx, 2, new QTableWidgetItem(com_buf));
      }
}

/*
 * Implement the WorkFolder items using FITS files. An image is a
 * fits file with a single HDU that contains the data array.
 */
WorkFolder::Image::Image(WorkFolder*folder, const QString&name)
: WorkFits(folder, name)
{
}

WorkFolder::Image::Image(WorkFolder*folder, const QString&name, const QFileInfo&file)
: WorkFits(folder, name, file)
{
}

WorkFolder::Image::~Image()
{
}
int WorkFolder::Image::copy_from_array(DataArray*src)
{
      DataArray::type_t src_type = src->get_type();
      vector<long> axes = src->get_axes();

      bool rc_bool = reconfig(axes, src_type);
      qassert(rc_bool);

      int rc;
      if (src_type == DT_VOID) {
	    rc = 0;

      } else if (src_type == DT_UINT8) {
	    uint8_t*buf = new uint8_t[axes[0]];
	    rc = do_copy_lines_(src, buf, TBYTE);

      } else if (src_type == DT_UINT16) {
	    uint16_t*buf = new uint16_t[axes[0]];
	    rc = do_copy_lines_(src, buf, TUSHORT);

      } else if (src_type == DT_INT16) {
	    int16_t*buf = new int16_t[axes[0]];
	    rc = do_copy_lines_(src, buf, TSHORT);

      } else if (src_type == DT_UINT32) {
	    uint32_t*buf = new uint32_t[axes[0]];
	    rc = do_copy_lines_(src, buf, TULONG);

      } else if (src_type == DT_INT16) {
	    int32_t*buf = new int32_t[axes[0]];
	    rc = do_copy_lines_(src, buf, TLONG);

      } else {
	    qinternal_error("Source type not handled here.");
	    rc = -1;
      }

      return rc;
}

vector<long> WorkFolder::Image::get_axes(void) const
{
      if (fd_ == 0)
	    return vector<long>();

      int status = 0;
      int naxis = 0;

      fits_get_img_dim(fd_, &naxis, &status);

      vector<long>res (naxis);
      fits_get_img_size(fd_, naxis, &res[0], &status);

      return res;
}

DataArray::type_t WorkFolder::Image::get_type(void) const
{
      int status = 0;
      int bitpix = 0;
      fits_get_img_equivtype(fd_, &bitpix, &status);

      switch (bitpix) {
	  case BYTE_IMG: return DT_UINT8;
	  case SBYTE_IMG: return DT_INT8;
	  case SHORT_IMG: return DT_INT16;
	  case USHORT_IMG: return DT_UINT16;
	  case LONG_IMG: return DT_INT32;
	  case ULONG_IMG: return DT_UINT32;
	  case LONGLONG_IMG: return DT_INT64;
	  case FLOAT_IMG: return DT_FLOAT32;
	  case DOUBLE_IMG: return DT_FLOAT64;
	  default: return DT_VOID;
      }
}

bool WorkFolder::Image::reconfig(const std::vector<long>&axes, DataArray::type_t type)
{
	// Can only reconfig new Image items
      if (fd_ != 0)
	    return false;

      int status = 0;

      QFileInfo img_path (folder()->work_dir(), getDisplayName() + "-i.fits");
      qassert(! img_path.exists());

      QString path_str = img_path.filePath();
      fits_create_diskfile(&fd_, path_str.toLocal8Bit().constData(), &status);
      if (status != 0) {
	    show_fits_error_stack(path_str);
	    return -1;
      }

      qassert(fd_);

      int bitpix = 0;
      switch (type) {
	  case DT_VOID:
	    break;
	  case DT_UINT8:
	    bitpix = BYTE_IMG;
	    break;
	  case DT_INT8:
	    bitpix = SBYTE_IMG;
	    break;
	  case DT_UINT16:
	    bitpix = USHORT_IMG;
	    break;
	  case DT_INT16:
	    bitpix = SHORT_IMG;
	    break;
	  case DT_UINT32:
	    bitpix = ULONG_IMG;
	    break;
	  case DT_INT32:
	    bitpix = LONG_IMG;
	    break;
	  case DT_INT64:
	    bitpix = LONGLONG_IMG;
	    break;
	  case DT_FLOAT32:
	    bitpix = FLOAT_IMG;
	    break;
	  case DT_FLOAT64:
	  case DT_DOUBLE:
	    bitpix = DOUBLE_IMG;
	    break;
      }
      qassert(bitpix != 0);

      vector<long>axes_tmp = axes;
      fits_create_img(fd_, bitpix, axes.size(), &axes_tmp[0], &status);

      return true;
}

int WorkFolder::Image::set_line_raw(const std::vector<long>&addr, long wid,
				    DataArray::type_t type, const void*data,
				    const uint8_t*alpha)
{
      qassert(get_type() == type);

      int status = 0;

	// Make sure the axes counts are correct. (cfitsio will not
	// check this, and instead will gleefully run past the end of
	// the fpixel array...)
      int naxes = 0;
      fits_get_img_dim(fd_, &naxes, &status);
      qassert(naxes==(int)addr.size());

      int datatype = 0;
      switch (type) {
	  case DataArray::DT_UINT16:
	    datatype = TUSHORT;
	    break;
	  default:
	    qinternal_error("Source type not handled here");
	    break;
      }

      vector<long> addr_tmp = addr;
      for (size_t idx = 0 ; idx < addr_tmp.size() ; idx += 1)
	    addr_tmp[idx] += 1;

      if (alpha == 0) {
	    fits_write_pix(fd_, datatype, &addr_tmp[0], wid, (void*)data, &status);
	    qassert(status == 0);
	    if (status != 0) {
		  show_fits_error_stack("WorkFolder::Image::set_line_raw");
	    }
	    return 0;
      }

      if (type == DT_UINT16) {
	    uint16_t*buf = new uint16_t[wid];
	    int16_t fits_blank = 0x7fff;
	    uint16_t use_blank = 0xffff;
	    int status = 0;
	    fits_update_key(fd_, TSHORT, "BLANK", &fits_blank, 0, &status);
	    fits_set_imgnull(fd_, fits_blank, &status);
	    qassert_fits_status(status);
	    return do_set_line_alpha_(addr_tmp, wid, buf, TUSHORT, use_blank, data, alpha);
      }

      qinternal_error("WorkFits::Image::set_line_raw not implemented");
      return -1;
}

int WorkFolder::Image::get_line_raw(const std::vector<long>&addr, long wid,
				    type_t type, void*data,
				    int&has_alpha, uint8_t*alpha)
{
      int rc = 0;
      vector<long> axes = get_axes();
      qassert(axes.size() == addr.size());

      vector<long>fpixel (addr.size());
      for (size_t idx = 0 ; idx < addr.size() ; idx += 1)
	    fpixel[idx] = addr[idx] + 1;

      int status = 0;
      int anynul = 0;
      vector<char> nulmask (wid);

      switch (type) {
	  case DataArray::DT_UINT8:
	    fits_read_pixnull(fd_, TBYTE, &fpixel[0], wid,
			      data, &nulmask[0], &anynul, &status);
	    break;
	  case DataArray::DT_UINT16:
	    fits_read_pixnull(fd_, TUSHORT, &fpixel[0], wid,
			      data, &nulmask[0], &anynul, &status);
	    break;

	  default:
	    QMessageBox::warning(0, "WorkFolder::Image::get_line_raw",
				 "Unsupported data type?");
	    rc = -1;
	    break;
      }

      if (anynul) {
	    for (int idx = 0 ; idx < wid ; idx += 1) {
		  if (alpha) alpha[idx] = nulmask[idx]? 0x00 : 0xff;
		  if (nulmask[idx]) has_alpha += 1;
	    }
      } else {
	    if (alpha) memset(alpha, 0xff, wid);
	    has_alpha = 0;
      }

      return rc;
}

template<class T>int WorkFolder::Image::do_copy_lines_(DataArray*src,
						       T*buf, int datatype)
{
      vector<long> src_axes = src->get_axes();

      vector<long> dst_addr = DataArray::zero_addr(src_axes.size());
      vector<long> fits_addr (dst_addr.size());

      int status = 0;

      do {
	    int has_alpha = 0;
	    int rc = src->get_line(dst_addr, src_axes[0], buf, has_alpha, 0);
	    qassert(rc >= 0);

	    for (size_t idx = 0 ; idx < fits_addr.size() ; idx += 1)
		  fits_addr[idx] = dst_addr[idx] + 1;

	    fits_write_pix(fd_, datatype, &fits_addr[0], src_axes[0], buf, &status);

      } while (DataArray::incr(dst_addr, src_axes, 1));

      qassert(status == 0);
      if (status != 0) {
	    show_fits_error_stack("Copy lines");
      }

      fits_flush_file(fd_, &status);
      return 0;
}

template<class T>int WorkFolder::Image::do_set_line_alpha_(std::vector<long>&axes, long wid, T*buf, int datatype, T blank_val, const void*data, const uint8_t*alpha)
{
      const T*use_data = reinterpret_cast<const T*> (data);
      for (long idx = 0 ; idx < wid ; idx += 1) {
	    if (alpha[idx] == 0)
		  buf[idx] = blank_val;
	    else
		  buf[idx] = use_data[idx];
      }

      int status = 0;
      fits_write_pixnull(fd_, datatype, &axes[0], wid, buf, &blank_val, &status);
      qassert_fits_status(status);

      delete[]buf;

      return 0;
}

QWidget* WorkFolder::Image::create_view_dialog(QWidget*dialog_parent)
{
      if (fd_ == 0)
	    return 0;

      QImage image;
      int status = 0;
      int naxis = 0;
      fits_get_img_dim(fd_, &naxis, &status);

      switch (naxis) {
	  case 0: // NULL image
	  case 1: // vector
	    break;
	  case 2: // 2D image
	    render_chdu_into_qimage(fd_, 1, 1, 1, image, &status);
	    break;
	  case 3: // 3D image; treat is as 2D with chroma.
	    render_chdu_into_qimage(fd_, 1, 2, 3, image, &status);
	    break;
	  default: // N-dimensional data cube. Just pick a plane and
		   // render that.
	    render_chdu_into_qimage(fd_, 1, 1, 1, image, &status);
	    break;
      }

      if (status != 0) {
	    char status_str[FLEN_STATUS];
	    fits_get_errstatus(status, status_str);
	    QMessageBox::warning(0, "FITS (cfitsio) error", status_str);
      }

      return new SimpleImageView(dialog_parent, image, getDisplayName());
}


WorkFolder::Table::Table(WorkFolder*folder, const QString&name)
: WorkFits(folder, name)
{
}

WorkFolder::Table::Table(WorkFolder*folder, const QString&name, const QFileInfo&file)
: WorkFits(folder, name, file)
{
      qassert(fd_);

	// A workspace table FITS file is expected to have two HDUs,
	// an empty primary and the table extension.
      int status = 0;
      int hdunum = 0;
      fits_get_num_hdus(fd_, &hdunum, &status);
      qassert(hdunum >= 2);

      int hdutype = 0;
      fits_movabs_hdu(fd_, 2, &hdutype, &status);
      qassert(hdutype == ASCII_TBL || hdutype == BINARY_TBL);
}

WorkFolder::Table::~Table()
{
}

int WorkFolder::Table::create_table(vector<DataTable::column_t>&info)
{
      if (fd_ != 0) return -1;

      int status = 0;

      QFileInfo img_path (folder()->work_dir(), getDisplayName() + "-t.fits");
      qassert(! img_path.exists());
      QString path_str = img_path.filePath();
      fits_create_diskfile(&fd_, path_str.toLocal8Bit().constData(), &status);
      if (status != 0) {
	    show_fits_error_stack(path_str);
	    return -1;
      }

      qassert(fd_);

      vector<char*> ttype (info.size());
      vector<char*> tform (info.size());
      for (size_t idx = 0 ; idx < info.size() ; idx += 1) {
	    DataTable::column_t&cur = info[idx];

	    ttype[idx] = strdup(cur.heading.toAscii().constData());
	    switch (cur.type) {
		case DT_CHAR:
		  tform[idx] = strdup("A");
		  break;
		case DT_UINT8:
		  tform[idx] = strdup("B");
		  break;
		case DT_INT16:
		  tform[idx] = strdup("I");
		  break;
		case DT_INT32:
		  tform[idx] = strdup("J");
		  break;
		case DT_DOUBLE:
		  tform[idx] = strdup("D");
		  break;
		case DT_STRING:
		  qassert(cur.max_elements > 0);
		  tform[idx] = strdup(QString("PA(%1)").arg(cur.max_elements).toAscii().constData());
		  break;
		default:
		  qinternal_error(QString("Type code %1 not implemented in column %2") .arg(cur.type) .arg(idx));
		  tform[idx] = "A";
		  break;
	    }
      }

      fits_create_tbl(fd_, BINARY_TBL, 0, info.size(), &ttype[0], &tform[0],
		      0, 0, &status);

      for (size_t idx = 0 ; idx < info.size() ; idx += 1) {
	    free(ttype[idx]);
	    free(tform[idx]);
      }

      preview_view_changed();
      return 0;
}

int WorkFolder::Table::set_value_int32(size_t row, size_t col, int32_t val)
{
      if (fd_ == 0) return -1;
      if (col >= table_cols()) return -1;

	// The row number allows for extending the table by one row to
	// accomodate a new value. So allow a row number below the
	// last.  The fits_write_col function will automaticall extend
	// the table if needed.
      if (row > table_rows()) return -1;

      int status = 0;
      fits_write_col(fd_, TINT32BIT, col+1, row+1, 1, 1, &val, &status);
      if (status != 0) {
	    show_fits_error_stack(QString("Error writing int32 at row=%1, col=%2").arg(row).arg(col));
	    return -1;
      }

      preview_view_changed();
      return 0;
}

int WorkFolder::Table::set_value_double(size_t row, size_t col, double val)
{
      if (fd_ == 0) return -1;
      if (col >= table_cols()) return -1;
      if (row > table_rows()) return -1;

      int status = 0;
      fits_write_col(fd_, TDOUBLE, col+1, row+1, 1, 1, &val, &status);
      if (status != 0) {
	    show_fits_error_stack(QString("Error writing double at row=%1, col=%2").arg(row).arg(col));
	    return -1;
      }

      preview_view_changed();
      return 0;
}

int WorkFolder::Table::set_value_string(size_t row, size_t col, const QString&val)
{
      if (fd_ == 0) return -1;
      if (col >= table_cols()) return -1;

	// The row number allows for extending the table by one row to
	// accomodate a new value. So allow a row number below the
	// last.  The fits_write_col function will automaticall extend
	// the table if needed.
      if (row > table_rows()) return -1;

      char val_buf[512+1];
      strncpy(val_buf, val.toLocal8Bit().constData(), sizeof val_buf);

      int status = 0;
      char*val_ptr[1];
      val_ptr[0] = val_buf;
      fits_write_col(fd_, TSTRING, col+1, row+1, 1, 1, val_ptr, &status);
      if (status != 0) {
	    show_fits_error_stack(QString("Error writing string at row=%1, col=%2").arg(row).arg(col));
	    return -1;
      }

      preview_view_changed();
      return 0;
}

size_t WorkFolder::Table::table_cols()
{
      if (fd_ == 0)
	    return 0;

      int status = 0;
      int ncols = 0;
      fits_get_num_cols(fd_, &ncols, &status);
      return ncols;
}

size_t WorkFolder::Table::table_rows()
{
      if (fd_ == 0)
	    return 0;

      int status = 0;
      long nrows = 0;
      fits_get_num_rows(fd_, &nrows, &status);
      return nrows;
}

DataTable::column_t WorkFolder::Table::table_col_info(size_t col)
{
      column_t res;
      if (fd_ == 0)
	    return res;
      if (col >= table_cols())
	    return res;

      int status = 0;
      int typecode = 0;
      long repeat = 0;
      long width = 0;
      fits_get_coltype(fd_, col+1, &typecode, &repeat, &width, &status);

      char key_buf[FLEN_KEYWORD];
      QString key_name = QString("%1").arg(col+1);
      strncpy(key_buf, key_name.toAscii().constData(), sizeof key_buf);

      char ttype[FLEN_VALUE];
      int colnum = -1;
      fits_get_colname(fd_, CASESEN, key_buf, ttype, &colnum, &status);
      qassert((size_t)colnum == col+1);

      res.heading = ttype;
      res.repeat = repeat;
      switch (typecode) {
	  case TINT32BIT:
	    res.type = DT_INT32;
	    break;
	  case -TSTRING: // Variable length array of char
	    res.type = DT_STRING;
	    break;
	  case TDOUBLE:
	    res.type = DT_DOUBLE;
	    break;
	  default:
	    qinternal_error(QString("Unhandled type code %1").arg(typecode));
	    break;
      }

      return res;
}

uint8_t WorkFolder::Table::table_value_uint8(size_t row, size_t col)
{
      qassert(fd_ && col < table_cols() && row < table_rows());

      int status = 0;
      uint8_t val = 0;
      fits_read_col(fd_, TBYTE, col+1, row+1, 0, 1, 0,
		    &val, 0, &status);
      qassert(status == 0);

      return val;
}

int16_t WorkFolder::Table::table_value_int16(size_t row, size_t col)
{
      qassert(fd_ && col < table_cols() && row < table_rows());

      int status = 0;
      int16_t val = 0;
      fits_read_col(fd_, TSHORT, col+1, row+1, 0, 1, 0,
		    &val, 0, &status);
      qassert(status == 0);

      return val;
}

int32_t WorkFolder::Table::table_value_int32(size_t row, size_t col)
{
      qassert(fd_ && col < table_cols() && row < table_rows());

      int status = 0;
      int32_t val = 0;
      fits_read_col(fd_, TINT32BIT, col+1, row+1, 1, 1, 0,
		    &val, 0, &status);
      if (status != 0) show_fits_error_stack(QString("Error reading int32 at row=%1, col=%2").arg(row).arg(col));

      return val;
}

double WorkFolder::Table::table_value_double(size_t row, size_t col)
{
      qassert(fd_ && col < table_cols() && row < table_rows());

      int status = 0;
      double val = 0.0;
      fits_read_col(fd_, TDOUBLE, col+1, row+1, 1, 1, 0,
		    &val, 0, &status);
      if (status != 0) show_fits_error_stack(QString("Error reading double at row=%1, col=%2").arg(row).arg(col));

      return val;
}

QString WorkFolder::Table::table_value_string(size_t row, size_t col)
{
      qassert(fd_ && col < table_cols() && row < table_rows());

	// Strings are stored as variable-length arrays of ASCII. Get
	// from the descriptor the size of this array, so that we can
	// know how big a buffer is needed.
      int status = 0;
      long repeat = 0;
      long offset = 0;
      fits_read_descript(fd_, col+1, row+1, &repeat, &offset, &status);

      char*val[1];
      val[0] = new char[repeat+1];

	// Read the value into the buffer, then convert it to a string.
      fits_read_col(fd_, TSTRING, col+1, row+1, 1, 1, 0, val, 0, &status);
      if (status != 0) {
	    show_fits_error_stack(QString("Error reading string at row=%1, col=%2").arg(row).arg(col));
      }

      QString res = val[0];

	// Clean up and return the result.
      delete[]val[0];
      return res;
}

QWidget* WorkFolder::Table::create_view_dialog(QWidget*dialog_parent)
{
      return new SimpleTableView(dialog_parent, this, getDisplayName());
}
