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
# include  <QFileDialog>
# include  <vector>
# include  "qassert.h"

using namespace std;

int FitsbenchMain::ftcl_choose_file_thunk_(ClientData raw, Tcl_Interp*interp,
					   int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_choose_file_(objc, objv);
}

int FitsbenchMain::ftcl_choose_file_(int objc, Tcl_Obj*const objv[])
{
      static QString image_filter
	    (tr("FITS Data files (*.fit *.fits *.fts)"
		";;PNM Images (*.pgm *.ppm)"
		";;TIFF Images (*.tif *.tiff)"
		";;Any (*.pgm *.ppm *.tif *.tiff *.fit *.fits *.fts)"));
      static QString image_caption (tr("Select image files."));

      QString filter, caption;

      for (int idx = 1 ; idx < objc ; idx += 1) {
	    if (strcmp(Tcl_GetString(objv[idx]),"-i") == 0) {
		  filter = image_filter;
		  caption = image_caption;
	    }
      }

      QStringList files = QFileDialog::getOpenFileNames(this, caption, QString(), filter);

      vector<Tcl_Obj*> file_list (files.size());

      for (int idx = 0 ; idx < files.size() ; idx += 1) {
	    QString str = files[idx];
	    Tcl_Obj*tmp = Tcl_NewStringObj(str.toStdString().c_str(), str.size());
	    file_list[idx] = tmp;
      }

      Tcl_Obj*file_list_obj = Tcl_NewListObj(file_list.size(), &file_list[0]);
      Tcl_SetObjResult(tcl_engine_, file_list_obj);

      return TCL_OK;
}
