/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_canvases.h
**	\brief Template Header
**
**	$Id$
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

#ifndef __SYNFIG_STUDIO_DIALOG_CANVASES_H
#define __SYNFIG_STUDIO_DIALOG_CANVASES_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treeview.h>
#include <gui/docks/dockable.h>
#include <gui/instance.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dock_Canvases : public Dockable
{
	Gtk::TreeView *canvas_tree;
	//Gtk::Menu	menu;
	etl::loose_handle<studio::Instance>	selected_instance;

private:

	void set_selected_instance_(etl::handle<studio::Instance> x);

	etl::loose_handle<studio::Instance> get_selected_instance() { return selected_instance; }

	etl::loose_handle<synfig::Canvas> get_selected_canvas();

	etl::loose_handle<studio::CanvasView> get_selected_canvas_view();

	void set_selected_instance(etl::loose_handle<studio::Instance> x);

	void set_selected_instance_signal(etl::handle<studio::Instance> x);

	void new_instance(etl::handle<studio::Instance> x);

	void delete_instance(etl::handle<studio::Instance> x);

	void refresh_instances();

	bool close();
	//! Signal handler of signal_row_activated, look for the desired canvas, and
	//! give it the focus
	/*! \see studio::Instance::focus */
	void on_row_activate(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *);
	//bool on_tree_event(GdkEvent *event);



	void on_action_toggle(const Glib::ustring& path);
	Gtk::Widget* create_canvas_tree();

public:

	Dock_Canvases();
	~Dock_Canvases();
}; // END of Dock_Canvases

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
