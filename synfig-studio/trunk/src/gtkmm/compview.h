/*! ========================================================================
** Sinfg
** Template Header File
** $Id: compview.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_COMPVIEW_H
#define __SINFG_COMPVIEW_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/menu.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/notebook.h>
#include <gtkmm/treeview.h>
#include <gtkmm/box.h>

#include <ETL/handle>

#include "dialogsettings.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg { class Canvas; };

namespace studio {

class Instance;
class CanvasView;

class CompView : public Gtk::Window
{
	DialogSettings dialog_settings;
	
	Gtk::Tooltips tooltips;

	Gtk::OptionMenu *instance_selector;
	Gtk::Notebook *notebook;

	Gtk::TreeView *canvas_tree;
	Gtk::TreeView *action_tree;

	Gtk::Menu	instance_list_menu;
	Gtk::Menu	menu;

	etl::loose_handle<studio::Instance>	selected_instance;

	void set_selected_instance_(etl::handle<studio::Instance> x);

	void clear_history();
	void clear_redo();
	
public:
	CompView();
	~CompView();

	etl::loose_handle<studio::Instance> get_selected_instance() { return selected_instance; }

	etl::loose_handle<sinfg::Canvas> get_selected_canvas();

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
	Gtk::Widget* create_instance_selector();
	
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
