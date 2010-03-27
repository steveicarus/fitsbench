#ifndef __FitsbenchItem_H
#define __FitsbenchItem_H
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
# include  <QTreeWidgetItem>
# include  <QFileInfo>
# include  <vector>
# include  <fitsio.h>
# include  "Previewer.h"

class QTableWidget;

/*
 * A FitsbenchItem is a top-level item in the bench. The main property
 * for this item is that it has a unique name.
 */
class FitsbenchItem  : public QTreeWidgetItem {

    public:
      FitsbenchItem(const QString&name);
      virtual ~FitsbenchItem() =0;

    private:
      QString name_;
};

class BenchFile : public FitsbenchItem {

      class Path_ : public QTreeWidgetItem, public QFileInfo {

	  public:
	    explicit Path_(const QFileInfo&path);
	    ~Path_();
      };

    public:
      explicit BenchFile(const QString&name, const QFileInfo&path);
      ~BenchFile();

      QString filePath() const { return path_->filePath(); }

    private:
      Path_*path_;
};


class FitsFile : public BenchFile {

      class HDU : public QTreeWidgetItem, public Previewer {
	  public:
	    explicit HDU(FitsFile*parent, int num);
	    ~HDU();

	    void preview_into_stack(QStackedWidget*);

	  private:
	    int hdu_num_;
	    QTableWidget*preview_;
      };

    public:
      explicit FitsFile(const QString&name, const QFileInfo&path);
      ~FitsFile();

    public:
	// CFITSIO-like methods (See the cfitsio documentation)

	// Move to absolute HDU
      int movabs_hdu(int hdu_num, int&hdu_type, int&status);
      int get_hdrspace(int&nkeys, int&morekeys, int&status);
      int read_keyn(int keynum, QString&key, QString&val, QString&com, int&status);

    private:
      fitsfile*fd_;

      std::vector<HDU*> hdu_table_;
};

#endif
