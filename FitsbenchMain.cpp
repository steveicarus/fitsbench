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
# include  <QInputDialog>
# include  <QLineEdit>
# include  <QMenu>
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
	      SIGNAL(itemClicked(QTreeWidgetItem*,int)),
	      SLOT(bench_tree_clicked_slot_(QTreeWidgetItem*,int)));
      connect(ui.bench_tree,
	      SIGNAL(itemActivated(QTreeWidgetItem*,int)),
	      SLOT(bench_tree_activated_slot_(QTreeWidgetItem*,int)));
      connect(ui.bench_tree,
	      SIGNAL(customContextMenuRequested(const QPoint&)),
	      SLOT(bench_tree_custom_menu_slot_(const QPoint&)));

      connect(ui.commands_line,
	      SIGNAL(returnPressed()),
	      SLOT(commands_line_slot_()));

      tcl_engine_ = Tcl_CreateInterp();

      Tcl_CreateObjCommand(tcl_engine_, "bench",   &ftcl_bench_thunk_,   this, 0);
      Tcl_CreateObjCommand(tcl_engine_, "axes",    &ftcl_axes_thunk_,    this, 0);
      Tcl_CreateObjCommand(tcl_engine_, "scratch", &ftcl_scratch_thunk_, this, 0);
}

FitsbenchMain::~FitsbenchMain()
{
      Tcl_DeleteInterp(tcl_engine_);
}

void FitsbenchMain::set_bench_script_name_(FitsbenchItem*item, const QString&name)
{
      QString old_name = item->getScriptName();

      item->setScriptName(name);

      if (! old_name.isNull()) {
	    std::string tmp = old_name.toStdString();
	    script_names_.erase(tmp);
      }

      if (! name.isNull() && ! name.isEmpty()) {
	    std::string name_str = name.toStdString();
	    script_names_[name_str] = item;
      }
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
      QString filter (tr("Images (*.pgm *.ppm)"));

      QStringList files = QFileDialog::getOpenFileNames(this, tr("Select image files to open."),
							start_dir, filter);

      for (int idx = 0 ; idx < files.size() ; idx += 1) {
	    QFileInfo path = files.at(idx);
	    BenchFile*item = new PnmFile(path.fileName(), path);
	    ui.bench_tree->addTopLevelItem(item);
      }
}

/*
 * When the user clicks on an item that can be previewed in the
 * preview stack, have that item preview itself. Save the "activated"
 * signal for displaying a rendering of the image.
 */
void FitsbenchMain::bench_tree_clicked_slot_(QTreeWidgetItem*item, int)
{
      Previewer*view = dynamic_cast<Previewer*> (item);
      if (view == 0) return;
      view->preview_into_stack(ui.preview_stack);
}

void FitsbenchMain::bench_tree_activated_slot_(QTreeWidgetItem*item, int)
{
      Previewer*view = dynamic_cast<Previewer*> (item);
      if (view == 0) return;
      view->render_into_dialog(this);
}

void FitsbenchMain::bench_tree_custom_menu_slot_(const QPoint&pos)
{
      QTreeWidgetItem*raw_item = ui.bench_tree->itemAt(pos);
      if (raw_item == 0) return;

      FitsbenchItem*item = dynamic_cast<FitsbenchItem*> (raw_item);
      if (item == 0) return;

      QAction prev ("Preview", 0);
      QAction view ("Quick View", 0);
      QAction name ("Script Name", 0);
      QAction clos ("Close", 0);
      clos.setEnabled(false); // Not implemented yet

      QList<QAction*> menu_list;
      menu_list .append(&prev);
      menu_list .append(&view);
      menu_list .append(&name);
      menu_list .append(&clos);

      QAction*hit = QMenu::exec(menu_list, mapToGlobal(pos), &prev);

      if (hit == &prev) {
	    bench_tree_clicked_slot_(item, 0);
      } else if (hit == &view) {
	    bench_tree_activated_slot_(item, 0);
      } else if (hit == &name) {
	    QString text = item->getScriptName();
	    text = QInputDialog::getText(this, tr("Input Name"), 
					 tr("Enter a unique script name"),
					 QLineEdit::Normal, text);
	    if (! text.isNull()) set_bench_script_name_(item, text);

      } else if (hit == &clos) {
	    cerr << "XXXX Close" << endl;
      }
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
