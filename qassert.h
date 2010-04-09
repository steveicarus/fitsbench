#ifndef __selfcheck_H
#define __selfcheck_H
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

# include  <cassert>
# include  <QMessageBox>
# include  <QString>

/*
 * This is a GUI-ish version of assert that raises a warning box and
 * gives the user the option of continuing or aborting.
 */
# define qassert(expr) \
      do {								\
	    if (! (expr)) {						\
		  QString text = QString("Function %1...\n%2:%3: %4")	\
			.arg(__PRETTY_FUNCTION__)			\
			.arg(__FILE__)					\
			.arg(__LINE__)					\
			.arg(__STRING(expr));				\
		  QMessageBox::StandardButton btn;			\
		  btn = QMessageBox::warning(0, "Assertion Failed", text, QMessageBox::Abort|QMessageBox::Ignore, QMessageBox::Abort); \
		  if (btn == QMessageBox::Abort) abort();		\
	    }								\
      } while(0)

#endif
