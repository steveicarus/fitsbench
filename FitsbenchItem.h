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
# include  <tiffio.h>
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

      class HDU : public FitsbenchItem, public Previewer, public DataArray {
	  public:
	    explicit HDU(FitsFile*parent, int num);
	    ~HDU();

	  public: // Implementations for DataArray
	    virtual std::vector<long> get_axes(void) const;
	    type_t get_type(void) const;

	    int get_line_raw(const std::vector<long>&addr, long wid,
			     type_t type, void*data,
			     int&has_alpha, uint8_t*alpha);

	  protected: // Implementations for Previewer
	    void fill_in_info_table(QTableWidget*);
	    QWidget*create_view_dialog(QWidget*parent);

	  private:
	    int hdu_num_;
      };

    public:
      explicit FitsFile(const QString&name, const QFileInfo&path);
      ~FitsFile();

	// Render the image of the current HDU into the QImage. If it
	// is a 2D image, then render it as a grayscale image. If it
	// is 3D, then the red, green and blue integers are indexes
	// into the third dimension to select planes for an RGB
	// rendering. Use FITS conventions for plane numberings,
	// i.e. the first plane is 1, the second 2, etc.
	// The status is the cfitsio status.
      void render_chdu(QImage&image, int red, int green, int blu, int&status);

	// Get a line of image data from the current HDU. Use FITS
	// conventions for pixel numberings, i.e. start from 1 instead
	// of start from 0.
	// The status is the cfitsio status.
      int get_line_chdu(const std::vector<long>&addr, long wid,
			DataArray::type_t type, void*data,
			int&has_alpha, uint8_t*alpha, int&status);

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
	// Get image type
      int get_img_type(int&bitpix, int&status);
      int get_img_equivtype(int&bitpix, int&status);

    private:
      fitsfile*fd_;

      std::vector<HDU*> hdu_table_;
};

class PnmFile : public BenchFile {

      class HDU : public FitsbenchItem, public Previewer, public DataArray {
	  public:
	    explicit HDU(PnmFile*parent);
	    ~HDU();

	  public: // Implementations for DataArray
	    std::vector<long> get_axes(void) const;
	    type_t get_type(void) const;

	    int get_line_raw(const std::vector<long>&addr, long wid,
			     type_t pixtype, void*data,
			     int&has_alpha, uint8_t*alpha);

	  protected: // Implementations for Previewer
	    void fill_in_info_table(QTableWidget*);
	    QWidget*create_view_dialog(QWidget*parent);

      };

    public:
      explicit PnmFile(const QString&name, const QFileInfo&path);
      ~PnmFile();

      size_t width() const { return wid_; }
      size_t height() const { return hei_; }
      size_t planes() const { return pla_; }
      long  datamax() const { return max_; }

      void render_image(QImage&image);

	// Bytes per value
      size_t bpv() const { return max_ >= 256? 2 : 1; }

	// The HDU implementation of get_line_raw redirects here.
      int get_line_raw(const std::vector<long>&addr, long wid,
		       DataArray::type_t pixtype, void*data);

    private:
      void render_gray8_(QImage&image);
      void render_gray16_(QImage&image);
      void render_rgb8_(QImage&image);
      void render_rgb16_(QImage&image);

    private:
      FILE*fd_;

      HDU*hdu_;

      QSysInfo::Endian file_endian_; // Endian-ness of the image
      size_t wid_;
      size_t hei_;
      size_t pla_; // Pixel planes (1 or 3)
      long max_;
      fpos_t data_; // Offset into the file where the image data lives

      long cache_y_;
      uint8_t*cache_;
};

class TiffFile : public BenchFile {

      class HDU : public FitsbenchItem, public Previewer, public DataArray {
	  public:
	    explicit HDU(TiffFile*parent, tdir_t dirnum);
	    ~HDU();

	  public: // Implementations for DataArray
	    std::vector<long> get_axes(void) const;
	    type_t get_type(void) const;

	    int get_line_raw(const std::vector<long>&addr, long wid,
			     type_t pixtype, void*data,
			     int&has_alpha, uint8_t*alpha);

	  protected: // Implementations for Previewer
	    void fill_in_info_table(QTableWidget*);
	    QWidget*create_view_dialog(QWidget*parent);

	  private:
	    template <class T> int get_line_buf(long x, long wid, T*dst);

	  private:
	    tdir_t dirnum_;
	    type_t type_;
	    std::vector<long> axes_;

	    long cache_y_;
	    unsigned char* cache_;
      };

    public:
      explicit TiffFile(const QString&name, const QFileInfo&path);
      ~TiffFile();

      void render_image(QImage&image, TiffFile::HDU*ptr);

    public:
	// TiffFile verisons of tiffio functions.
      int readScanline(tdata_t buf, uint32 row, tsample_t sample);
      tsize_t scanlineSize(void);
      int setDirectory(tdir_t dirnum);
      template <class T> int getFieldDefaulted(ttag_t tag, T&val);

    private:
      template <class T> void do_render_image_gray(QImage&image, uint32 wid, uint32 hei, T*buf);

    private:
      TIFF*fd_;
      std::vector<HDU*> hdu_table_;

};

/*
 * Scratch images are work areas that the script can use to receive
 * processing results. These images are read/write.
 */
class ScratchImage  : public FitsbenchItem, public Previewer, public DataArray {

    public:
      ScratchImage (const QString&display_name);
      ~ScratchImage();

      void reconfig(const std::vector<long>&axes, DataArray::type_t type);

    public: // Implementations for DataArray
      std::vector<long> get_axes(void) const;
      DataArray::type_t get_type(void) const;

      int set_line_raw(const std::vector<long>&addr, long wid,
		       DataArray::type_t type, const void*data);

      int set_line_alpha(const std::vector<long>&addr, long wid, const uint8_t*data);

      virtual int get_line_raw(const std::vector<long>&addr, long wid,
			       type_t type, void*data,
			       int&has_alpha, uint8_t*alpha);

    private:
      template <class T> int do_set_line_(size_t off, long wid, const T*data);
      template <class T> int do_get_line_(size_t off, long wid, T*data);

    protected: // Implementations for Previewer
      void fill_in_info_table(QTableWidget*);
      QWidget*create_view_dialog(QWidget*parent);

    private:
      QWidget*create_view_double_(QWidget*dialog_parent, const double*array);
      QWidget*create_view_uint8_(QWidget*dialog_parent,  const uint8_t*array);
      QWidget*create_view_uint16_(QWidget*dialog_parent, const uint16_t*array);

    private:
      std::vector<long> axes_;
      DataArray::type_t type_;

      size_t addr_to_offset_(const std::vector<long>&addr) const;

      template <class T> T*get_array_(void);
      void delete_by_type_(void);
      union {
	    uint8_t*array_uint8_;
	    uint16_t*array_uint16_;
	    uint32_t*array_uint32_;
	    double*array_dbl_;
      };

	// Optional alpha mask for all the pixels of the image.
      uint8_t*alpha_;
};

/*
 * A WorkFolder is a persistent collection of items. The folder is
 * stored in the host file system as a directory. The items in the
 * collection are stored as files. We work with files in order to
 * avoid size constraints, and also to make the data persistent.
 */
class WorkFolder  : public BenchFile {

    public:
      WorkFolder (const QString&name, const QDir&path);
      ~WorkFolder();

    private:
};

#endif
