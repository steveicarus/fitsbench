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

# include  "FitsbenchItem.h"
# include  "SimpleImageView.h"
# include  <QStackedWidget>
# include  <QTableWidget>
# include  <iostream>
# include  <assert.h>

using namespace std;

FitsbenchItem::FitsbenchItem(const QString&name)
: name_(name)
{
      setText(0, name_);
}

FitsbenchItem::~FitsbenchItem()
{
}

BenchFile::BenchFile(const QString&name, const QFileInfo&path)
: FitsbenchItem(name)
{
      path_ = new Path_ (path);
      addChild(path_);
}

BenchFile::~BenchFile()
{
}


BenchFile::Path_::Path_(const QFileInfo&path)
: QFileInfo(path)
{
      setText(0, filePath());
}

BenchFile::Path_::~Path_()
{
}
