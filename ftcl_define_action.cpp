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
# include  <QInputDialog>
# include  <iostream>
# include  "qassert.h"

using namespace std;

int FitsbenchMain::ftcl_define_action_thunk_(ClientData raw, Tcl_Interp*interp,
					     int objc, Tcl_Obj*CONST objv[])
{
      FitsbenchMain*eng = reinterpret_cast<FitsbenchMain*> (raw);
      assert(eng->tcl_engine_ == interp);
      return eng->ftcl_define_action_(objc, objv);
}

int FitsbenchMain::ftcl_define_action_(int objc, Tcl_Obj*const objv[])
{
      QString name;
      QString script;

      if (objc == 1) {
	    name = QInputDialog::getText(this, tr("Action Name"),
					 tr("Enter a name for this action"),
					 QLineEdit::Normal);
	    script = QInputDialog::getText(this, tr("Action Script"),
					   tr("Enter the command for this action"));
      } else if (objc == 3) {
	    name = QString(Tcl_GetString(objv[1]));
	    script = QString(Tcl_GetString(objv[2]));
      } else {
	    Tcl_AppendResult(tcl_engine_, "Usage: define_action <text> <script>", 0);
	    return TCL_ERROR;
      }


      QAction*action = new QAction(name, this);
      action->setData(script);
      ui.menuActions->addAction(action);

      return TCL_OK;
}

void FitsbenchMain::defined_action_slot_(QAction*action)
{
      QVariant data = action->data();
      if (data.type() != QVariant::String) {
	    return;
      }

      QString script = data.toString();
      if (script.isEmpty())
	    return;

      run_command_string_(script);
}
