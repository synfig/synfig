/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_history.h
**	\brief Template Header
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

#ifndef __SYNFIG_STUDIO_DIALOG_HISTORY_H
#define __SYNFIG_STUDIO_DIALOG_HISTORY_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treeview.h>
#include <gui/instance.h>
#include <gui/docks/dock_canvasspecific.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dock_History : public Dock_CanvasSpecific
{
	Gtk::TreeView *action_tree;

	etl::loose_handle<studio::Instance>	selected_instance;

	sigc::connection on_undo_tree_changed_connection;

	void on_undo_tree_changed();

	void set_selected_instance_(etl::handle<studio::Instance> x);


	void set_selected_instance(etl::loose_handle<studio::Instance> x);

	void set_selected_instance_signal(etl::handle<studio::Instance> x);

	void delete_instance(etl::handle<studio::Instance> x);

	Gtk::Widget* create_action_tree();

public:

	etl::loose_handle<studio::Instance> get_selected_instance() { return selected_instance; }

	void clear_undo();
	void clear_redo();
	void clear_undo_and_redo();

	void update_undo_redo();

	Dock_History();
	~Dock_History();
protected:
	void init_instance_vfunc(etl::loose_handle<Instance> instance) override;
	void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view) override;

	bool on_action_event(GdkEvent *event);
	void on_action_toggle(const Glib::ustring& path);

}; // END of Dock_History

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
