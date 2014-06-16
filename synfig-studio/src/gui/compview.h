/* === S Y N F I G ========================================================= */
/*!	\file compview.h
**	\brief Header File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_COMPVIEW_H
#define __SYNFIG_COMPVIEW_H

/* === H E A D E R S ======================================================= */

#include <vector>

#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/tooltip.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/menu.h>
#include <gtkmm/notebook.h>
#include <gtkmm/treeview.h>
#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>

#include <ETL/handle>

#include "dialogsettings.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig { class Canvas; };

namespace studio {

class Instance;
class CanvasView;

class CompView : public Gtk::Window
{
	DialogSettings dialog_settings;

	Gtk::Tooltip tooltip;

	Gtk::ComboBoxText instance_selector;
	Gtk::Notebook *notebook;

	Gtk::TreeView *canvas_tree;
	Gtk::TreeView *action_tree;

	Gtk::Menu	menu;

	std::vector< etl::loose_handle<studio::Instance> > instances;
	etl::loose_handle<studio::Instance>	selected_instance;

	void set_selected_instance_(etl::handle<studio::Instance> x);

	void clear_history();
	void clear_redo();

protected:
	void on_instance_selector_changed();

public:
	CompView();
	~CompView();

	etl::loose_handle<studio::Instance> get_selected_instance() { return selected_instance; }

	etl::loose_handle<synfig::Canvas> get_selected_canvas();

	etl::loose_handle<studio::CanvasView> get_selected_canvas_view();

	void set_selected_instance(etl::loose_handle<studio::Instance> x);

	void set_selected_instance_signal(etl::handle<studio::Instance> x);

	void new_instance(etl::handle<studio::Instance> x);

	void delete_instance(etl::handle<studio::Instance> x);

	void refresh_instances();

	bool close();

private:

	Gtk::Widget* create_canvas_tree();
	Gtk::Widget* create_action_tree();

	void on_row_activate(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *);
	bool on_tree_event(GdkEvent *event);

	bool on_action_event(GdkEvent *event);

	void init_menu();

	void menu_new_canvas();
	void menu_delete();
	void menu_rename();

	void on_action_toggle(const Glib::ustring& path);
};

};

/* === E N D =============================================================== */

#endif
