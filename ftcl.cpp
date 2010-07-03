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
# include  <errno.h>
# include  "qassert.h"

using namespace std;

struct Tcl_ChannelType FitsbenchMain::tcl_stdout_type_ = {
      0,
      TCL_CHANNEL_VERSION_2,
      &FitsbenchMain::tcl_stdout_close_thunk_,
      &FitsbenchMain::tcl_stdout_input_thunk_,
      &FitsbenchMain::tcl_stdout_output_thunk_,
      0, // seekProc
      0, // setOptionProc
      0, // getOptionProc
      &FitsbenchMain::tcl_stdout_watch_thunk_,
      &FitsbenchMain::tcl_stdout_getHandle_thunk_,
      0, // close2Proc
      0, // blockModeProc
      0, // flushProc
      0, // handlerProc
      0, // wideSeekProc
      0  // threadActionProc
};

int FitsbenchMain::tcl_stdout_close_thunk_(ClientData data, Tcl_Interp*interp)
{
      FitsbenchMain*obj = reinterpret_cast<FitsbenchMain*> (data);
      return obj->tcl_stdout_closeProc_(interp);
}

int FitsbenchMain::tcl_stdout_input_thunk_(ClientData data, char*buf, int len, int*err)
{
      FitsbenchMain*obj = reinterpret_cast<FitsbenchMain*> (data);
      return obj->tcl_stdout_inputProc_(buf, len, err);
}

int FitsbenchMain::tcl_stdout_output_thunk_(ClientData data, CONST char*buf, int toWrite, int*err)
{
      FitsbenchMain*obj = reinterpret_cast<FitsbenchMain*> (data);
      return obj->tcl_stdout_outputProc_(buf, toWrite, err);
}

void FitsbenchMain::tcl_stdout_watch_thunk_(ClientData data, int mask)
{
      FitsbenchMain*obj = reinterpret_cast<FitsbenchMain*> (data);
      obj->tcl_stdout_watchProc_(mask);
}

int FitsbenchMain::tcl_stdout_getHandle_thunk_(ClientData data, int dir, ClientData*handlePtr)
{
      FitsbenchMain*obj = reinterpret_cast<FitsbenchMain*> (data);
      return obj->tcl_stdout_getHandleProc_(dir, handlePtr);
}

int FitsbenchMain::tcl_stdout_closeProc_(Tcl_Interp*)
{
      return 0;
}

int FitsbenchMain::tcl_stdout_inputProc_(char*, int, int*err)
{
      *err = EINVAL;
      return -1;
}

int FitsbenchMain::tcl_stdout_outputProc_(const char*buf, int toWrite, int*err)
{
      int res = 0;

      while (toWrite > 0) {
	    size_t trans = toWrite;
	    if ((trans + tcl_stdout_linebuf_fill_) >= sizeof (tcl_stdout_linebuf_))
		  trans = sizeof (tcl_stdout_linebuf_) - tcl_stdout_linebuf_fill_ - 1;

	    memcpy(tcl_stdout_linebuf_ + tcl_stdout_linebuf_fill_, buf, trans);
	    tcl_stdout_linebuf_fill_ += trans;
	    buf += trans;
	    toWrite -= trans;
	    res += trans;

	      // Eliminate any NULL bytes from the string, but make
	      // sure the buffer it NULL terminated.
	    for (size_t idx = 0 ; idx < tcl_stdout_linebuf_fill_ ; idx += 1) {
		  if (tcl_stdout_linebuf_[idx] == 0)
			tcl_stdout_linebuf_[idx] = '?';
	    }
	    tcl_stdout_linebuf_[tcl_stdout_linebuf_fill_] = 0;

	      // Write to the display log, a line at a time.
	    while (char*eol = strchr(tcl_stdout_linebuf_, '\n')) {
		  *eol++ = 0;

		  QString msg(tcl_stdout_linebuf_);
		  ui.commands_log->append(msg);

		  size_t len = eol - tcl_stdout_linebuf_;
		  memmove(tcl_stdout_linebuf_, eol, tcl_stdout_linebuf_fill_ - len);
		  tcl_stdout_linebuf_fill_ -= len;
	    }

	      // If the linebuf is full, then force an output even if
	      // the line isn't complete.
	    if (tcl_stdout_linebuf_fill_+1 == sizeof (tcl_stdout_linebuf_)) {
		  QString msg(tcl_stdout_linebuf_);
		  ui.commands_log->append(msg);
		  tcl_stdout_linebuf_fill_ = 0;
	    }
      }

      *err = 0;
      return res;
}

void FitsbenchMain::tcl_stdout_watchProc_(int)
{
}

int FitsbenchMain::tcl_stdout_getHandleProc_(int, ClientData*handlePtr)
{
      *handlePtr = 0;
      return TCL_ERROR;
}


FitsbenchItem* FitsbenchMain::item_from_name_(const string&name) const
{
      size_t slash_index = name.find('/');

	// If there are no slash ('/') charscters in the name, then
	// find a top level item.
      if (slash_index == string::npos) {

	    map<string,FitsbenchItem*>::const_iterator cur = script_names_.find(name);

	    if (cur == script_names_.end()) return 0;
	    else return cur->second;
      }

      QString folder_file;
      WorkFolder*folder = workfolder_from_name_(name.c_str(), folder_file);
      if (folder == 0)
	    return 0;

      return folder->find_item(folder_file);
}

FitsbenchItem* FitsbenchMain::item_from_name_(Tcl_Obj*obj) const
{
      string idx_nam = Tcl_GetString(obj);
      return item_from_name_(idx_nam);
}

WorkFolder::Image* FitsbenchMain::workitem_from_name_(Tcl_Obj*obj) const
{
      QString name;
      WorkFolder*folder = workfolder_from_name_(obj, name);
      if (folder == 0) return 0;
      return folder->get_image(name);
}

WorkFolder* FitsbenchMain::workfolder_from_name_(const QString&path, QString&nam) const
{
      if (path.count('/') != 1)
	    return 0;

      int slash_idx = path.indexOf('/');
      if (slash_idx == 0 || slash_idx == path.size()-1)
	    return 0;

      string folder_name = path.left(slash_idx).toStdString();
      nam = path.mid(slash_idx+1);

      FitsbenchItem* folder_item = item_from_name_(folder_name);
      if (folder_item == 0) return 0;

      WorkFolder*folder = dynamic_cast<WorkFolder*> (folder_item);
      if (folder == 0) return 0;

      return folder;
}

WorkFolder* FitsbenchMain::workfolder_from_name_(Tcl_Obj*obj, QString&nam) const
{
      QString path = Tcl_GetString(obj);
      return workfolder_from_name_(path, nam);
}

DataArray* FitsbenchMain::image_from_name_(const string&nam)
{
      size_t slash_index = nam.find('/');

	// If there are no slach ('/') characters in the name, then
	// find/create a top level item.
      if (slash_index == string::npos) {
	      // If the item already exists, return that.
	    FitsbenchItem*item = item_from_name_(nam);
	    if (item)
		  return dynamic_cast<DataArray*> (item);

	      // Otherwise, create a ScratchImage.
	    ScratchImage*dst = new ScratchImage(QString(nam.c_str()));
	    ui.bench_tree->addTopLevelItem(dst);
	    set_bench_script_name_(dst, QString(nam.c_str()));
	    return dst;
      }

      QString folder_file;
      WorkFolder*folder = workfolder_from_name_(nam.c_str(), folder_file);
      if (folder == 0)
	    return 0;

      return folder->get_image(folder_file);
}

DataArray* FitsbenchMain::image_from_name_(Tcl_Obj*obj)
{
      string nam = Tcl_GetString(obj);
      return image_from_name_(nam);
}

vector<long> FitsbenchMain::vector_from_listobj_(Tcl_Obj*obj)
{
      Tcl_Obj**axes_objv = 0;
      int axes_objc = 0;
      Tcl_ListObjGetElements(tcl_engine_, obj, &axes_objc, &axes_objv);

      vector<long> axes (axes_objc);
      for (int idx = 0 ; idx < axes_objc ; idx += 1) {
	    long tmp;
	    Tcl_GetLongFromObj(tcl_engine_, axes_objv[idx],  &tmp);
	    axes[idx] = tmp;
      }

      return axes;
}

Tcl_Obj* FitsbenchMain::listobj_from_vector_(const vector<long>&axes)
{
      Tcl_Obj*obj = Tcl_NewListObj(0, 0);

      for (size_t idx = 0 ; idx < axes.size() ; idx += 1) {
	    Tcl_Obj*cur = Tcl_NewLongObj(axes[idx]);
	    Tcl_ListObjAppendElement(tcl_engine_, obj, cur);
      }

      return obj;
}

Tcl_Obj* FitsbenchMain::listobj_from_vector_(const vector<double>&axes)
{
      Tcl_Obj*obj = Tcl_NewListObj(0, 0);

      for (size_t idx = 0 ; idx < axes.size() ; idx += 1) {
	    Tcl_Obj*cur = Tcl_NewDoubleObj(axes[idx]);
	    Tcl_ListObjAppendElement(tcl_engine_, obj, cur);
      }

      return obj;
}

int FitsbenchMain::ftcl_axes_thunk_(ClientData raw, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_axes_(objc, objv);
}

int FitsbenchMain::ftcl_minmax_thunk_(ClientData raw, Tcl_Interp*interp,
				      int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_minmax_(objc, objv);
}

int FitsbenchMain::ftcl_axes_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 2) {
	    Tcl_AppendResult(tcl_engine_, "Missing name.", 0);
	    return TCL_ERROR;
      }

      const char*name = Tcl_GetString(objv[1]);
      if (name == 0)
	    return TCL_ERROR;

      FitsbenchItem*item_raw = item_from_name_(name);
      if (item_raw == 0)
	    return TCL_ERROR;

      DataArray*item = dynamic_cast<DataArray*>(item_raw);
      if (item == 0)
	    return TCL_ERROR;

      vector<long> axes = item->get_axes();

      Tcl_Obj*res = Tcl_NewObj();
      for (size_t idx = 0 ; idx < axes.size() ; idx += 1) {
	    Tcl_Obj*val = Tcl_NewLongObj(axes[idx]);
	    Tcl_ListObjAppendElement(tcl_engine_, res, val);
      }

      Tcl_SetObjResult(tcl_engine_, res);

      return TCL_OK;
}

template <class T> static void minmax_in_line(const T*buf, long wid,
					      const vector<long>&addr,
					      T&min_val, vector<long>&min_pnt,
					      T&max_val, vector<long>&max_pnt)
{
      if (min_pnt.size() == 0) {
	    min_pnt = addr;
	    min_val = buf[0];
      }
      if (max_pnt.size() == 0) {
	    max_pnt = addr;
	    max_val = buf[0];
      }

      for (int idx = 0 ; idx < wid ; idx += 1) {
	    if (buf[idx] > max_val) {
		  max_val = buf[idx];
		  max_pnt = addr;
		  max_pnt[0] = idx;
	    }
	    if (buf[idx] < min_val) {
		  min_val = buf[idx];
		  min_pnt = addr;
		  min_pnt[0] = idx;
	    }
      }
}

int FitsbenchMain::ftcl_minmax_(int objc, Tcl_Obj*const objv[])
{
      if (objc < 2) {
	    Tcl_AppendResult(tcl_engine_, "Missing image argument.", 0);
	    return TCL_ERROR;
      }

      FitsbenchItem*item_raw = item_from_name_(objv[1]);
      if (item_raw == 0) {
	    Tcl_AppendResult(tcl_engine_, "Named argument not found.", 0);
	    return TCL_ERROR;
      }

      DataArray*item = dynamic_cast<DataArray*> (item_raw);
      if (item == 0) {
	    Tcl_AppendResult(tcl_engine_, "Named argument is not a data array", 0);
	    return TCL_ERROR;
      }

      if (item->get_type() == DataArray::DT_VOID)
	    return TCL_OK;

      vector<long> axes = item->get_axes();
      vector<long> addr = DataArray::zero_addr(axes.size());

      vector<long> min_pnt;
      vector<long> max_pnt;

      Tcl_Obj*res_obj[4];

      int has_alpha = 0;

      if (item->get_type() == DataArray::DT_UINT8) {
	    uint8_t min_val = 0xff, max_val = 0x00;
	    uint8_t*buf = new uint8_t[axes[0]];
	    do {
		  int rc = item->get_line(addr, axes[0], buf, has_alpha);
		  qassert(rc >= 0);
		  qassert(has_alpha == 0);
		  minmax_in_line(buf, axes[0], addr, min_val, min_pnt, max_val, max_pnt);
	    } while (DataArray::incr(addr, axes, 1));
	    delete[]buf;

	    res_obj[0] = Tcl_NewLongObj(min_val);
	    res_obj[2] = Tcl_NewLongObj(max_val);

      } else if (item->get_type() == DataArray::DT_UINT16) {
	    uint16_t min_val = 0xffff, max_val = 0x0000;
	    uint16_t*buf = new uint16_t[axes[0]];
	    do {
		  int rc = item->get_line(addr, axes[0], buf, has_alpha);
		  qassert(rc >= 0);
		  qassert(has_alpha == 0);
		  minmax_in_line(buf, axes[0], addr, min_val, min_pnt, max_val, max_pnt);
	    } while (DataArray::incr(addr, axes, 1));
	    delete[]buf;

	    res_obj[0] = Tcl_NewLongObj(min_val);
	    res_obj[2] = Tcl_NewLongObj(max_val);

      } else {
	    Tcl_AppendResult(tcl_engine_, "Unknown data array type.", 0);
	    return TCL_ERROR;
      }

      res_obj[1] = listobj_from_vector_(min_pnt);
      res_obj[3] = listobj_from_vector_(max_pnt);

      Tcl_SetObjResult(tcl_engine_, Tcl_NewListObj(4, res_obj));
      return TCL_OK;
}
