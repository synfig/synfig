/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_pluginmanager.h
**	\brief Plugin Manager Dialog
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_DIALOG_PLUGINMANAGER_H
#define __SYNFIG_GTKMM_DIALOG_PLUGINMANAGER_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>

#include <gtkmm/dialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/notebook.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/listbox.h>
#include <gui/pluginmanager.h>
/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class Dialog_PluginManager : public Gtk::Dialog 
{
public:
    Dialog_PluginManager(Gtk::Window& parent);
    ~Dialog_PluginManager();

private:
    Gtk::ListBox plugin_list_box;
    Gtk::FileChooserDialog plugin_file_dialog;
    Gtk::MessageDialog message_dialog;
    Gtk::Notebook notebook;

    std::vector<studio::Plugin> plugin_list;
    void build_listbox();
    void on_install_plugin_button_clicked();
    void refresh();

}; // END of class Dialog_PluginManager

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
