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

class FitsbenchMain : public QMainWindow {

      Q_OBJECT

    public:
      FitsbenchMain(QWidget*parent =0);
      ~FitsbenchMain();

    private:
	// The user interface...
      Ui::FitsbenchMainWidget ui;
	// The TCL engine...
      Tcl_Interp*tcl_engine_;

    private slots:
	// Menu actions
      void action_OpenFITS_slot_(void);
      void action_OpenImage_slot_(void);

	// Slots to handle widget signals
      void bench_tree_clicked_slot_(QTreeWidgetItem*, int);
      void bench_tree_activated_slot_(QTreeWidgetItem*, int);
      void commands_line_slot_(void);

};

#endif
