#ifndef __ChooseOne_H
#define __ChooseOne_H
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

# include  <QDialog>
# include  "ui_choose_one.h"

class ChooseOne : public QDialog {

      Q_OBJECT

    public:
      ChooseOne(QWidget*parent, const QString&text, const QStringList&selections);
      ~ChooseOne();

    public:
      static QString select(QWidget*parent, const QString&text, const QStringList&selections);

    private slots:
      void selection_slot_(const QString&text);

    private:
      Ui::ChooseOneDialog ui;

      QString cur_selection_;
};

#endif
