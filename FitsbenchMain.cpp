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
# include  <QCloseEvent>
# include  <QFileDialog>
# include  <QInputDialog>
# include  <QLineEdit>
# include  <QMenu>
# include  <QMessageBox>
# include  <QTreeWidget>
# include  <iostream>
# include  "qassert.h"

using namespace std;

const FitsbenchMain::ftcl_command_table FitsbenchMain::ftcl_commands[] = {
      { "axes",            &FitsbenchMain::ftcl_axes_thunk_ },
      { "bayer_decompose", &FitsbenchMain::ftcl_bayer_decomp_thunk_ },
      { "bench",           &FitsbenchMain::ftcl_bench_thunk_ },
      { "choose_one",      &FitsbenchMain::ftcl_choose_one_thunk_ },
      { "copy",            &FitsbenchMain::ftcl_copy_thunk_ },
      { "crop",            &FitsbenchMain::ftcl_crop_thunk_ },
      { "define_action",   &FitsbenchMain::ftcl_define_action_thunk_ },
      { "folder",          &FitsbenchMain::ftcl_folder_thunk_ },
      { "minmax",          &FitsbenchMain::ftcl_minmax_thunk_ },
      { "normalize",       &FitsbenchMain::ftcl_normalize_thunk_ },
      { "phase_correlate", &FitsbenchMain::ftcl_phase_corr_thunk_ },
      { "pixbin",          &FitsbenchMain::ftcl_pixbin_thunk_ },
      { "scratch",         &FitsbenchMain::ftcl_scratch_thunk_ },
      { "stack",           &FitsbenchMain::ftcl_stack_thunk_ },
      { "table",           &FitsbenchMain::ftcl_table_thunk_ },
      { 0, 0}
};

FitsbenchMain::FitsbenchMain(QWidget*parent)
: QMainWindow(parent)
{
      ui.setupUi(this);

      connect(ui.actionFITS_File,
	      SIGNAL(triggered()),
	      SLOT(action_FITS_File_slot_()));
      connect(ui.actionFITSBench_Work_Folder,
	      SIGNAL(triggered()),
	      SLOT(action_FITSBench_Work_Folder_slot_()));
      connect(ui.actionOpenImage,
	      SIGNAL(triggered()),
	      SLOT(action_OpenImage_slot_()));
      connect(ui.actionOpenFITSBench_Work_Folder,
	      SIGNAL(triggered()),
	      SLOT(action_OpenFITSBench_Work_Folder_slot_()));
      connect(ui.actionOpen_TCL_Script,
	      SIGNAL(triggered()),
	      SLOT(action_Open_TCL_Script_slot_()));

      ui.actionDefine_Action->setData(QString("define_action"));

      connect(ui.menuActions,
	      SIGNAL(triggered(QAction*)),
	      SLOT(defined_action_slot_(QAction*)));

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

      tcl_stdout_linebuf_fill_ = 0;
      tcl_engine_ = Tcl_CreateInterp();
      tcl_stdout_ = Tcl_CreateChannel(&tcl_stdout_type_, "console", this, TCL_WRITABLE);
      Tcl_RegisterChannel(tcl_engine_, tcl_stdout_);
      Tcl_SetChannelOption(tcl_engine_, tcl_stdout_, "-buffering", "line");
      Tcl_SetStdChannel(tcl_stdout_, TCL_STDOUT);


      for (int idx = 0 ; ftcl_commands[idx].name ; idx += 1)
	    Tcl_CreateObjCommand(tcl_engine_, ftcl_commands[idx].name,
				 ftcl_commands[idx].thunk,
				 this, 0);
}

FitsbenchMain::~FitsbenchMain()
{
      Tcl_DeleteInterp(tcl_engine_);
}

void FitsbenchMain::closeEvent(QCloseEvent*event)
{
      event->accept();
}

void FitsbenchMain::set_bench_script_name_(FitsbenchItem*item, const QString&name)
{
      QString old_name = item->getScriptName();

      item->setScriptName(name);

      if (! old_name.isNull()) {
	    std::string tmp = old_name.toStdString();
	    script_names_.erase(tmp);
      }

      if (! name.isEmpty()) {
	    std::string name_str = name.toStdString();

	    if (FitsbenchItem*old_item = script_names_[name_str])
		  old_item->setScriptName("");

	    script_names_[name_str] = item;
      }
}

void FitsbenchMain::clear_bench_script_names_(FitsbenchItem*item)
{
      set_bench_script_name_(item, QString());

      for (int idx = 0 ; idx < item->childCount() ; idx += 1) {
	    QTreeWidgetItem*cur_raw = item->child(idx);
	    if (FitsbenchItem*cur = dynamic_cast<FitsbenchItem*> (cur_raw))
		  clear_bench_script_names_(cur);
      }
}

void FitsbenchMain::bench_tree_delete_item_(FitsbenchItem*item)
{
      int item_index = ui.bench_tree->indexOfTopLevelItem(item);
      qassert(item_index >= 0);

      clear_bench_script_names_(item);
      QTreeWidgetItem*tmp = ui.bench_tree->takeTopLevelItem(item_index);
      assert(tmp == item);
      delete item;
}

void FitsbenchMain::action_OpenImage_slot_(void)
{
      QString start_dir;
      QString filter (tr("FITS Data files (*.fit *.fits *.fts)"
			 ";;PNM Images (*.pgm *.ppm)"
			 ";;TIFF Images (*.tif *.tiff)"
			 ";;Any (*.pgm *.ppm *.tif *.tiff *.fit *.fits *.fts)"));

      QStringList files = QFileDialog::getOpenFileNames(this, tr("Select image files to open."),
							start_dir, filter);

      for (int idx = 0 ; idx < files.size() ; idx += 1) {
	    QFileInfo path = files.at(idx);
	    QString suff = path.completeSuffix();

	    BenchFile*item = 0;
	    if (suff=="fit" || suff=="fits" || suff=="fts") {
		  item = new FitsFile(path.fileName(), path);
	    } else if (suff=="pgm" || suff=="ppm") {
		  item = new PnmFile(path.fileName(), path);
	    } else if (suff=="tif" || suff=="tiff") {
		  item = new TiffFile(path.fileName(), path);
	    }

	    if (item) ui.bench_tree->addTopLevelItem(item);
      }
}

/*
 * Open an existing FITSBench Work Folder. Ask the user for an
 * existing directory, and use that directory to create the WorkFolder
 * object.
 */
void FitsbenchMain::action_OpenFITSBench_Work_Folder_slot_(void)
{
      QString fname = QFileDialog::getExistingDirectory(0,
                    tr("Select FITSBench Work Folder."));
      if (fname.isEmpty())
	  return;

      QDir path = fname;
      qassert(path.exists());

      WorkFolder*item = new WorkFolder(path.dirName(), path);
      ui.bench_tree->addTopLevelItem(item);

      QString script_name = item->getScriptName();
      if (! script_name.isEmpty())
	    set_bench_script_name_(item, script_name);
}

void FitsbenchMain::action_Open_TCL_Script_slot_(void)
{
      QString start_dir;
      QString filter (tr("TCL Scripts (*.tcl)"));

      QStringList files = QFileDialog::getOpenFileNames(this, tr("Select TCL scripts to open."),
							start_dir, filter);

      for (int idx = 0 ; idx < files.size() ; idx += 1) {
	    QFileInfo path = files.at(idx);

	    int tcl_rc = Tcl_EvalFile(tcl_engine_, path.filePath().toStdString().c_str());
	    QString msg (Tcl_GetStringResult(tcl_engine_));
	    if  (! msg.isEmpty()) {
		  QColor save_color = ui.commands_log->textColor();
		  QColor use_color = save_color;
		  if (tcl_rc != TCL_OK) use_color = QColor(255,0,0);

		  ui.commands_log->setTextColor(use_color);
		  ui.commands_log->append(msg);
		  ui.commands_log->setTextColor(save_color);
	    }
      }
}

void FitsbenchMain::action_FITS_File_slot_(void)
{
      QMessageBox::information(this, tr("New FITS File"),
			       "New FITS File not implemented yet.");
}

/*
 * Create a new FITSBench Work Folder. Prompt the user for a new file
 * name, create a directory from that file name, and create a
 * WorkFolder object to reference that directory.
 */
void FitsbenchMain::action_FITSBench_Work_Folder_slot_(void)
{
      QString fname = QFileDialog::getSaveFileName(0,
		    tr("New FITSBench Work Folder"),
		    QString(), QString(), 0);
      if (fname.isEmpty())
	    return;

      QFileInfo path = fname;

      if (path.exists()) {
	    QMessageBox::information(0, tr("File Exists"),
		            QString("File exists and it not a directory"));
	    return;
      }

      QDir path_dir = path.dir();

      if (! path_dir.mkdir(path.fileName())) {
	    QMessageBox::information(0, tr("ERROR"),
			    QString("Unable to make directory %1 in %2")
			    .arg(path.fileName()) .arg(path_dir.path()));
	    return;
      }

      path_dir.cd(path.fileName());

      WorkFolder*item = new WorkFolder(path_dir.dirName(), path_dir);
      ui.bench_tree->addTopLevelItem(item);
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

      int item_index = ui.bench_tree->indexOfTopLevelItem(raw_item);
	// Close only works for top level items.
      if (item_index >= 0)
	    clos.setEnabled(true);
      else
	    clos.setEnabled(false);

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
	    bench_tree_delete_item_(item);
      }
}

void FitsbenchMain::run_command_string_(const QString&script)
{
      int save_weight = ui.commands_log->fontWeight();
      ui.commands_log->setFontWeight(QFont::Bold);
      ui.commands_log->append(script);
      ui.commands_log->setFontWeight(save_weight);

      int tcl_rc = Tcl_Eval(tcl_engine_, script.toStdString().c_str());
      QString msg (Tcl_GetStringResult(tcl_engine_));

      if (! msg.isEmpty()) {
	    QColor save_color = ui.commands_log->textColor();
	    QColor use_color = save_color;
	    if (tcl_rc != TCL_OK) use_color = QColor(255,0,0);

	    ui.commands_log->setTextColor(use_color);
	    ui.commands_log->append(msg);
	    ui.commands_log->setTextColor(save_color);
      }
}

void FitsbenchMain::commands_line_slot_(void)
{
      QString line = ui.commands_line->text();
      ui.commands_line->clear();
      run_command_string_(line);
}
