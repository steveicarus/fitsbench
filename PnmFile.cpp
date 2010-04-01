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

static int next_word(FILE*fd)
{
      for (;;) {
	    int chr = fgetc(fd);
	    if (chr == EOF)
		  return chr;

	    if (strchr(" \r\n", chr))
		  continue;

	    if (chr == '#') {
		  while (chr != '\n') {
			chr = fgetc(fd);
			if (chr == EOF)
			      return EOF;
		  }
		  continue;
	    }

	      // Not space, not a comment, must be the next word.
	    return chr;
      }
}

PnmFile::PnmFile(const QString&name, const QFileInfo&path)
: BenchFile(name, path)
{
      fd_ = fopen(filePath().toLocal8Bit().constData(), "rb");

      char chr;
      chr = fgetc(fd_);
      assert(chr == 'P');
      chr = fgetc(fd_);
      assert(chr == '5');
      pla_ = 1;

      chr = next_word(fd_);

      wid_ = 0;
      while (isdigit(chr)) {
	    wid_ *= 10;
	    wid_ += chr - '0';
	    chr = fgetc(fd_);
      }

      chr = next_word(fd_);

      hei_ = 0;
      while (isdigit(chr)) {
	    hei_ *= 10;
	    hei_ += chr - '0';
	    chr = fgetc(fd_);
      }

      chr = next_word(fd_);

      max_ = 0;
      while (isdigit(chr)) {
	    max_ *= 10;
	    max_ += chr - '0';
	    chr = fgetc(fd_);
      }

      while (chr != '\n' && chr != EOF)
	    chr = fgetc(fd_);

      fgetpos(fd_, &data_);

      hdu_ = new HDU(this);
}

PnmFile::~PnmFile()
{
      fclose(fd_);
}

PnmFile::HDU::HDU(PnmFile*par)
: FitsbenchItem(par)
{
      view_ = 0;
      preview_ = 0;

      setDisplayName("image");
}

PnmFile::HDU::~HDU()
{
      if (view_) delete view_;
      if (preview_) delete preview_;
}

vector<long> PnmFile::HDU::get_axes(void) const
{
      PnmFile*pnm = dynamic_cast<PnmFile*> (parent());
      assert(pnm);


      vector<long> res (pnm->planes()==1? 2 : 3);

      res[0] = pnm->width();
      res[1] = pnm->height();
      if (pnm->planes() != 1) res[2] = pnm->planes();

      return res;
}

void PnmFile::HDU::preview_into_stack(QStackedWidget*wstack)
{
      if (preview_ == 0) {
	    PnmFile*pnm = dynamic_cast<PnmFile*> (parent());
	    assert(pnm);

	    QStringList headers;
	    headers << QString("Keyword") << QString("Value");
	    preview_ = new QTableWidget(3, 2);
	    preview_->setHorizontalHeaderLabels(headers);

	    preview_->setItem(0, 0, new QTableWidgetItem("width"));
	    preview_->setItem(0, 1, new QTableWidgetItem(pnm->width()));

	    preview_->setItem(1, 0, new QTableWidgetItem("height"));
	    preview_->setItem(1, 1, new QTableWidgetItem(pnm->height()));

	    preview_->setItem(2, 0, new QTableWidgetItem("planes"));
	    preview_->setItem(3, 1, new QTableWidgetItem(pnm->planes()));

	    wstack->addWidget(preview_);
      }

      wstack->setCurrentWidget(preview_);
}

void PnmFile::HDU::render_into_dialog(QWidget*dial)
{
}