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

class QStackedWidget;

/*
 * Objects derived from Previewer can preview themselves in a
 * QStackedWidget by creating a widget that describes themselves,
 * adding that widget to the stack, and enabling the stack.
 */
class Previewer {

    public:
      Previewer();
      virtual ~Previewer() =0;

      virtual void preview_into_stack(QStackedWidget*) =0;

    private:

};

#endif
