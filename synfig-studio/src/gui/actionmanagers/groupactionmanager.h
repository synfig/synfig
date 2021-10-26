/* === S Y N F I G ========================================================= */
/*!	\file groupactionmanager.h
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

#ifndef __SYNFIG_GROUP_ACTION_MANAGER_H
#define __SYNFIG_GROUP_ACTION_MANAGER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/uimanager.h>
#include <synfigapp/canvasinterface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class LayerGroupTree;

class GroupActionManager
{
	Glib::RefPtr<Gtk::UIManager> ui_manager_;
	LayerGroupTree* group_tree_;
	etl::handle<synfigapp::CanvasInterface> canvas_interface_;

	Glib::RefPtr<Gtk::ActionGroup>	action_group_;
	Gtk::UIManager::ui_merge_id 	popup_id_;

	sigc::connection selection_changed_connection;

	bool queued;
	sigc::connection queue_refresh_connection;

private:

	void on_action_add();

public:
	void queue_refresh();

	GroupActionManager();
	~GroupActionManager();

	void set_ui_manager(const Glib::RefPtr<Gtk::UIManager> &x);
	Glib::RefPtr<Gtk::UIManager> get_ui_manager()const { return ui_manager_; }

	void set_group_tree(LayerGroupTree* x);
	LayerGroupTree* get_group_tree()const { return group_tree_; }

	void set_canvas_interface(const etl::handle<synfigapp::CanvasInterface> &x);
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const { return canvas_interface_; }

	void refresh();
	void clear();
}; // END of GroupActionManager

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
