/* === S Y N F I G ========================================================= */
/*!	\file docks/dockmanager.h
**	\brief Template Header
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_DOCKMANAGER_H
#define __SYNFIG_DOCKMANAGER_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <list>
#include <map>
#include <synfig/string.h>
#include <sigc++/sigc++.h>
#include <ETL/smart_ptr>

#include <gtkmm/widget.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dockable;
class DockBook;
class DockDialog;
class DockSettings;

class DockManager : public sigc::trackable
{
	friend class Dockable;
	friend class DockBook;
	friend class DockDialog;
	friend class DockSettings;

	std::list<Dockable*> dockable_list_;
	std::list<DockDialog*> dock_dialog_list_;

	sigc::signal<void,Dockable*> signal_dockable_registered_;
	sigc::signal<void,Dockable*> signal_dockable_unregistered_;

	etl::smart_ptr<DockSettings> dock_settings;

private:
	static std::map<Gtk::Container*, bool> containers_to_remove_;

	void write_separator(std::string &x, bool continue_ = true);
	void write_string(std::string &x, const std::string &str);
	void write_int(std::string &x, int i);
	void write_bool(std::string &x, bool b);
	void write_widget(std::string &x, Gtk::Widget* widget);

	bool         read_separator(std::string &x);
	std::string  read_string(std::string &x);
	int          read_int(std::string &x);
	bool         read_bool(std::string &x);
	Gtk::Widget* read_widget(std::string &x);

public:
	DockManager();
	~DockManager();

	DockDialog& find_dock_dialog(int id);
	const DockDialog& find_dock_dialog(int id)const;
	//! Connect to that signal to get the info a dock has been registered
	/*! \see studio::DockManager::register_dockable */
	sigc::signal<void,Dockable*>& signal_dockable_registered() { return signal_dockable_registered_; }
	//! Connect to that signal to get the info that a dock has been unregistered
	/*! \see studio::DockManager::unregister_dockable */
	sigc::signal<void,Dockable*>& signal_dockable_unregistered() { return signal_dockable_unregistered_; }

	void register_dockable(Dockable& x);
	bool unregister_dockable(Dockable& x);
	Dockable& find_dockable(const synfig::String& x);
	void present(synfig::String x);
	void show_all_dock_dialogs();

	std::string save_widget_to_string(Gtk::Widget *widget);
	Gtk::Widget* load_widget_from_string(const std::string &x);

	std::string save_layout_to_string();
	void load_layout_from_string(const std::string &x);

	void update_window_titles();

	static std::string layout_from_template(const std::string &tpl, float dx, float dy, float sx, float sy);

	void set_dock_area_visibility(bool visible, DockBook* source);

	static bool swap_widgets(Gtk::Widget &widget1, Gtk::Widget &widget2);
	static void remove_widget_recursive(Gtk::Widget &widget);
	static void remove_widget_by_pointer_recursive(Gtk::Widget *widget) { remove_widget_recursive(*widget); }
	static void remove_empty_container_recursive(Gtk::Container &container);
	static void remove_empty_container_by_pointer_recursive(Gtk::Container *container) { remove_empty_container_recursive(*container); }
	static bool add_widget(Gtk::Widget &dest_widget, Gtk::Widget &src_widget, bool vertical, bool first);
	static bool add_dockable(Gtk::Widget &dest_widget, Dockable &dockable, bool vertical, bool first);
}; // END of class DockManager

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
