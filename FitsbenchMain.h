#ifndef __FitsbenchMain_H
#define __FitsbenchMain_H
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

# include  <qapplication.h>
# include  "ui_fitsbench.h"
# include  <tcl.h>

class FitsbenchItem;

class FitsbenchMain : public QMainWindow {

      Q_OBJECT

    public:
      FitsbenchMain(QWidget*parent =0);
      ~FitsbenchMain();

    private:
	// The user interface...
      Ui::FitsbenchMainWidget ui;

	// For the TCL engine, keep a map of script names to
	// scriptables. The user manipulates this map by setting or
	// changing the "script name" for the bench item.
      std::map<std::string, FitsbenchItem*> script_names_;
      void set_bench_script_name_(FitsbenchItem*item, const QString&name);

	// Clear the script names for this item and all its children.
      void clear_bench_script_names_(FitsbenchItem*item);

	// Convenience function for getting an item from the script
	// name. If there is no item with the name, return 0.
      FitsbenchItem* item_from_name_(const std::string&nam) const;
      FitsbenchItem* item_from_name_(Tcl_Obj*obj) const;

    private:
	// The TCL engine...
      Tcl_Interp*tcl_engine_;

      std::vector<long> vector_from_listobj_(Tcl_Obj*obj);
      Tcl_Obj* listobj_from_vector_(const std::vector<long>&axes);

	// Implementations of TCL commands...
      int ftcl_bench_(int objc, Tcl_Obj*const objv[]);
      int ftcl_axes_(int objc, Tcl_Obj*const objv[]);
      int ftcl_bayer_decompose_(int objc, Tcl_Obj*const objv[]);
      int ftcl_crop_(int objc, Tcl_Obj*const objv[]);
      int ftcl_scratch_(int objc, Tcl_Obj*const objv[]);
      int ftcl_minmax_(int objc, Tcl_Obj*const objv[]);
      int ftcl_phase_corr_(int objc, Tcl_Obj*const objv[]);
      int ftcl_pixbin_(int objc, Tcl_Obj*const objv[]);

	// Stubs to convert the Tcl interpreter's call to the command
	// back to a method of this object.
      static int ftcl_bench_thunk_(ClientData obj, Tcl_Interp*interp,
				   int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_axes_thunk_(ClientData obj, Tcl_Interp*interp,
				  int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_bayer_decomp_thunk_(ClientData obj, Tcl_Interp*interp,
					  int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_crop_thunk_(ClientData obj, Tcl_Interp*interp,
				  int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_scratch_thunk_(ClientData obj, Tcl_Interp*interp,
				     int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_minmax_thunk_(ClientData obj, Tcl_Interp*interp,
				     int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_phase_corr_thunk_(ClientData obj, Tcl_Interp*interp,
					int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_pixbin_thunk_(ClientData obj, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[]);
      static const struct ftcl_command_table {
	    const char*name;
	    int (*thunk) (ClientData, Tcl_Interp*, int objc, Tcl_Obj*CONST objv[]);
      } ftcl_commands[];

    private slots:
	// Menu actions
      void action_OpenImage_slot_(void);
      void action_OpenFITSBench_Work_Folder_slot_(void);
      void action_FITS_File_slot_(void);
      void action_FITSBench_Work_Folder_slot_(void);

	// Slots to handle widget signals
      void bench_tree_clicked_slot_(QTreeWidgetItem*, int);
      void bench_tree_activated_slot_(QTreeWidgetItem*, int);
      void bench_tree_custom_menu_slot_(const QPoint&);

      void commands_line_slot_(void);

};

#endif
