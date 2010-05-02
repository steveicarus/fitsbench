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


ObjectAutoPtr::ObjectAutoPtr()
{
      target_ = 0;
}

ObjectAutoPtr::~ObjectAutoPtr()
{
      if (target_) delete target_;
}

ObjectAutoPtr& ObjectAutoPtr::operator = (QWidget*tgt)
{
      if (target_) delete target_;
      target_ = tgt;
      connect(target_, SIGNAL(destroyed(QObject*)), SLOT(target_destroyed(QObject*)));
      return *this;
}

void ObjectAutoPtr::destroy()
{
      if (target_) {
	    QWidget*tmp = target_;
	    target_ = 0;
	    delete tmp;
      }
}

void ObjectAutoPtr::target_destroyed(QObject*obj)
{
      if (obj == target_) target_ = 0;
}
