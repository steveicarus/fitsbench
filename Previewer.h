#ifndef __Previewer_H
#define __Previewer_H
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

# include  "ObjectAutoPtr.h"
class QStackedWidget;
class QTableWidget;
class QWidget;

/*
 * Objects derived from Previewer can preview themselves in a
 * QStackedWidget by creating a widget that describes themselves,
 * adding that widget to the stack, and enabling the stack.
 */
class Previewer  {

    public:
      Previewer();
      virtual ~Previewer() =0;

      void preview_into_stack(QStackedWidget*);
      void render_into_dialog(QWidget*parent);

    protected:
	// The derived class calls this to indicate that the contents
	// of a view may have changed. The previewer must redraw the
	// view if this happens.
      void preview_view_changed();

    protected:
	// The derived class implements this to refill the info table.
      virtual void fill_in_info_table(QTableWidget*) =0;
	// The derived class implements this to create a display dialog.
      virtual QWidget* create_view_dialog(QWidget*dialog_parent) =0;

    private:
      ObjectAutoPtr table_;
      ObjectAutoPtr view_;
};

#endif
