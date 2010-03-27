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
# include  "FitsbenchMain.h"
# include  "FitsbenchItem.h"
# include  <QFileDialog>
# include  <QLineEdit>
# include  <QTreeWidget>
# include  <iostream>

using namespace std;

FitsbenchMain::FitsbenchMain(QWidget*parent)
: QMainWindow(parent)
{
      ui.setupUi(this);

      connect(ui.actionOpenFITS,
	      SIGNAL(triggered()),
	      SLOT(action_OpenFITS_slot_()));
      connect(ui.actionOpenImage,
	      SIGNAL(triggered()),
	      SLOT(action_OpenImage_slot_()));

      connect(ui.bench_tree,
	      SIGNAL(itemActivated(QTreeWidgetItem*,int)),
	      SLOT(bench_tree_activated_slot_(QTreeWidgetItem*,int)));

      connect(ui.commands_line,
	      SIGNAL(returnPressed()),
	      SLOT(commands_line_slot_()));

      tcl_engine_ = Tcl_CreateInterp();
}

FitsbenchMain::~FitsbenchMain()
{
      Tcl_DeleteInterp(tcl_engine_);
}

void FitsbenchMain::action_OpenFITS_slot_(void)
{
      QString start_dir;
      QString filter (tr("FITS Data files (*.fit *.fits *.fts)"));

      QStringList files = QFileDialog::getOpenFileNames(this, tr("Select FITS files to open."),
							start_dir, filter);

      for (int idx = 0 ; idx < files.size() ; idx += 1) {
	    QFileInfo path = files.at(idx);
	    FitsFile*item = new FitsFile(path.fileName(), path);
	    ui.bench_tree->addTopLevelItem(item);
      }
}

void FitsbenchMain::action_OpenImage_slot_(void)
{
      QString start_dir;
      QString filter (tr("Images (*.png *.jpg *.tif *.tiff)"));

      QStringList files = QFileDialog::getOpenFileNames(this, tr("Select image files to open."),
							start_dir, filter);

      for (int idx = 0 ; idx < files.size() ; idx += 1) {
	    QFileInfo path = files.at(idx);
	    BenchFile*item = new BenchFile(path.fileName(), path);
	    ui.bench_tree->addTopLevelItem(item);
      }
}

void FitsbenchMain::bench_tree_activated_slot_(QTreeWidgetItem*item, int)
{
      Previewer*view = dynamic_cast<Previewer*> (item);
      if (view == 0) return;
      view->preview_into_stack(ui.preview_stack);
}

void FitsbenchMain::commands_line_slot_(void)
{
      QString line = ui.commands_line->text();
      ui.commands_line->clear();

      int save_weight = ui.commands_log->fontWeight();
      ui.commands_log->setFontWeight(QFont::Bold);
      ui.commands_log->append(line);
      ui.commands_log->setFontWeight(save_weight);

      int tcl_rc = Tcl_Eval(tcl_engine_, line.toStdString().c_str());
      QString msg (Tcl_GetStringResult(tcl_engine_));

      QColor save_color = ui.commands_log->textColor();
      QColor use_color = save_color;
      if (tcl_rc != TCL_OK) use_color = QColor(255,0,0);

      ui.commands_log->setTextColor(use_color);
      ui.commands_log->append(msg);
      ui.commands_log->setTextColor(save_color);
}
