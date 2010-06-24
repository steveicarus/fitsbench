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
# include  "FitsbenchItem.h"
# include  <tcl.h>


class FitsbenchMain : public QMainWindow {

      Q_OBJECT

    public:
      FitsbenchMain(QWidget*parent =0);
      ~FitsbenchMain();

    private:
	// The user interface...
      Ui::FitsbenchMainWidget ui;

      void closeEvent(QCloseEvent*);

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

	// Locate or create a DataArray with the given script name. If
	// the name is a work/path string, then create the image in
	// the WorkFolder. Otherwise, create a ScratchImage with the
	// same name.
      DataArray* image_from_name_(const std::string&nam);
      DataArray* image_from_name_(Tcl_Obj*obj);

	// Similar to above, but in this case the image *must* be of
	// the form work/file.
      WorkFolder::Image* workitem_from_name_(Tcl_Obj*obj) const;

	// Convenience function for getting the existing folder for
	// the name. The name part is returned to the nam argument.
      WorkFolder* workfolder_from_name_(const QString&path, QString&nam) const;
      WorkFolder* workfolder_from_name_(Tcl_Obj*obj, QString&nam) const;

    private:
	// The TCL engine...
      Tcl_Interp*tcl_engine_;
      Tcl_Channel tcl_stdout_;

      void run_command_string_(const QString&script);

      std::vector<long> vector_from_listobj_(Tcl_Obj*obj);
      Tcl_Obj* listobj_from_vector_(const std::vector<long>&axes);
      Tcl_Obj* listobj_from_vector_(const std::vector<double>&axes);

      static Tcl_ChannelType tcl_stdout_type_;
      char tcl_stdout_linebuf_[2048];
      size_t tcl_stdout_linebuf_fill_;

	// Imlementations of TCL I/O functions...
      int tcl_stdout_closeProc_(Tcl_Interp*);
      int tcl_stdout_inputProc_(char*, int, int*);
      int tcl_stdout_outputProc_(const char*buf, int toWrite, int*err);
      void tcl_stdout_watchProc_(int);
      int tcl_stdout_getHandleProc_(int, ClientData*);

      static int tcl_stdout_close_thunk_(ClientData, Tcl_Interp*);
      static int tcl_stdout_input_thunk_(ClientData, char*, int, int*);
      static int tcl_stdout_output_thunk_(ClientData, CONST char*, int, int*);
      static void tcl_stdout_watch_thunk_(ClientData, int);
      static int tcl_stdout_getHandle_thunk_(ClientData, int, ClientData*);

	// Implementations of TCL commands...
      int ftcl_bench_(int objc, Tcl_Obj*const objv[]);
      int ftcl_axes_(int objc, Tcl_Obj*const objv[]);
      int ftcl_bayer_decompose_(int objc, Tcl_Obj*const objv[]);
      int ftcl_choose_one_(int objc, Tcl_Obj*const objv[]);
      int ftcl_copy_(int objc, Tcl_Obj*const objv[]);
      int ftcl_crop_(int objc, Tcl_Obj*const objv[]);
      int ftcl_define_action_(int objc, Tcl_Obj*const objv[]);
      int ftcl_folder_(int objc, Tcl_Obj*const objv[]);
      int ftcl_scratch_(int objc, Tcl_Obj*const objv[]);
      int ftcl_minmax_(int objc, Tcl_Obj*const objv[]);
      int ftcl_normalize_(int objc, Tcl_Obj*const objv[]);
      int ftcl_phase_corr_(int objc, Tcl_Obj*const objv[]);
      int ftcl_pixbin_(int objc, Tcl_Obj*const objv[]);
      int ftcl_stack_(int objc, Tcl_Obj*const objv[]);
      int ftcl_table_(int objc, Tcl_Obj*const objv[]);

	// Stubs to convert the Tcl interpreter's call to the command
	// back to a method of this object.
      static int ftcl_bench_thunk_(ClientData obj, Tcl_Interp*interp,
				   int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_axes_thunk_(ClientData obj, Tcl_Interp*interp,
				  int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_bayer_decomp_thunk_(ClientData obj, Tcl_Interp*interp,
					  int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_choose_one_thunk_(ClientData obj, Tcl_Interp*interp,
					int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_copy_thunk_(ClientData obj, Tcl_Interp*interp,
				       int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_crop_thunk_(ClientData obj, Tcl_Interp*interp,
				  int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_define_action_thunk_(ClientData obj, Tcl_Interp*interp,
					   int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_folder_thunk_(ClientData obj, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_scratch_thunk_(ClientData obj, Tcl_Interp*interp,
				     int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_minmax_thunk_(ClientData obj, Tcl_Interp*interp,
				     int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_normalize_thunk_(ClientData obj, Tcl_Interp*interp,
				       int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_phase_corr_thunk_(ClientData obj, Tcl_Interp*interp,
					int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_pixbin_thunk_(ClientData obj, Tcl_Interp*interp,
				    int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_stack_thunk_(ClientData obj, Tcl_Interp*interp,
				   int objc, Tcl_Obj*CONST objv[]);
      static int ftcl_table_thunk_(ClientData obj, Tcl_Interp*interp,
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
      void action_Open_TCL_Script_slot_(void);

      void defined_action_slot_(QAction*);

	// Slots to handle widget signals
      void bench_tree_clicked_slot_(QTreeWidgetItem*, int);
      void bench_tree_activated_slot_(QTreeWidgetItem*, int);
      void bench_tree_custom_menu_slot_(const QPoint&);

      void commands_line_slot_(void);

};

#endif
