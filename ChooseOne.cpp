
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
# include  "ChooseOne.h"
# include  <iostream>

using namespace std;

ChooseOne::ChooseOne(QWidget*parent, const QString&text, const QStringList&sel)
: QDialog(parent)
{
      ui.setupUi(this);

      ui.label->setText(text);
      ui.selection->addItems(sel);
      cur_selection_ = ui.selection->currentText();

      connect(ui.selection,
	      SIGNAL(activated(const QString&)),
	      SLOT(selection_slot_(const QString&)));
}

ChooseOne::~ChooseOne()
{
}

void ChooseOne::selection_slot_(const QString&text)
{
      cur_selection_ = text;
}

QString ChooseOne::select(QWidget*parent, const QString&text, const QStringList&selections)
{
      ChooseOne dialog (parent, text, selections);
      int rc = dialog.exec();
      if (rc == QDialog::Rejected)
	    return QString();
      else
	    return dialog.cur_selection_;
}
