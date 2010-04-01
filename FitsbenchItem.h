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
# include  "DataArray.h"

class QTableWidget;
class SimpleImageView;

/*
 * A FitsbenchItem is a top-level item in the bench. The main property
 * for this item is that it has a display name and an optional script
 * name, and knows how to put them into the right columns. Use this
 * class instead of the QTreeWidgetItem for any objects that go into
 * the fits bench tree.
 */
class FitsbenchItem  : public QTreeWidgetItem {

    public:
      FitsbenchItem(FitsbenchItem*parent);
      FitsbenchItem(const QString&name);
      virtual ~FitsbenchItem() =0;

      void setDisplayName(const QString&txt) { setText(0, txt); }
      QString getDisplayName(void) const     { return text(0); }

      void setScriptName(const QString&txt)  { setText(1, txt); }
      QString getScriptName(void) const      { return text(1); }

      virtual std::vector<long> get_axes(void) const;

    private:
};

class BenchFile : public FitsbenchItem {

      class Path_ : public FitsbenchItem, public QFileInfo {

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

      class HDU : public FitsbenchItem, public Previewer {
	  public:
	    explicit HDU(FitsFile*parent, int num);
	    ~HDU();

	    virtual std::vector<long> get_axes(void) const;

	    void preview_into_stack(QStackedWidget*);
	    void render_into_dialog(QWidget*parent);

	  private:
	    int hdu_num_;
	    QTableWidget*preview_;
	    SimpleImageView*view_;
      };

    public:
      explicit FitsFile(const QString&name, const QFileInfo&path);
      ~FitsFile();

	// Render the image of the current HDU into the QImage. If it
	// is a 2D image, then render it as a grayscale image. If it
	// is 3D, then the red, green and blue integers are indexes
	// into the thrid dimension to select planes for an RGB
	// rendering. Use FITS conventions for plane numberings,
	// i.e. the first plane is 1, the second 2, etc.
      void render_chdu(QImage&image, int red, int green, int blu, int&status);

    public:
	// CFITSIO-like methods (See the cfitsio documentation)

	// Move to absolute HDU
      int movabs_hdu(int hdu_num, int&hdu_type, int&status);
	// Get number of keys in HDU
      int get_hdrspace(int&nkeys, int&morekeys, int&status);
	// Read key from HDU
      int read_keyn(int keynum, QString&key, QString&val, QString&com, int&status);
	// Get image dimensions
      int get_img_dim(int&naxis, int&status);
	// Get image size
      int get_img_size(std::vector<long>&naxes, int&status);

    private:
      fitsfile*fd_;

      std::vector<HDU*> hdu_table_;
};

/*
 * Scratch images are work areas that the script can use to receive
 * processing results. These images are read/write.
 */
class ScratchImage  : public FitsbenchItem, public DataArray {

    public:
      ScratchImage (const QString&display_name);
      ~ScratchImage();

      void reconfig(const std::vector<long>&axes, DataArray::type_t type);

      std::vector<long> get_axes(void) const;

    private:
      std::vector<long> axes_;
      DataArray::type_t type_;
};

#endif
