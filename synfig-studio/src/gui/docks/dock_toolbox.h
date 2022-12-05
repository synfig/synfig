/* === S Y N F I G ========================================================= */
/*!	\file dock_toolbox.h
**	\brief Header File
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
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_DOCK_TOOLBOX_H
#define __SYNFIG_GTKMM_DOCK_TOOLBOX_H

/* === H E A D E R S ======================================================= */

#include <gui/docks/dockable.h>
#include <gui/smach.h>

#include <gtkmm/radiotoolbutton.h>
#include <gtkmm/toolitemgroup.h>
#include <gtkmm/box.h>

#include <map>

#include <synfig/string.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk {
class Paned;
}

namespace studio {

class StateManager;

class Dock_Toolbox : public Dockable
{
	friend class studio::StateManager;

	Gtk::ToolItemGroup *tool_item_group;
	Gtk::Paned *tool_box_paned;

	// Tool/state name => {Tool button, Smach::state}
	std::map<synfig::String, std::pair<Gtk::RadioToolButton *, const void*>> state_button_map;

	bool changing_state_;

	Gtk::RadioToolButton::Group radio_tool_button_group;

	void on_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);

	void change_state_(const Smach::state_base *state);

	void update_tools();

	void set_active_state(const synfig::String& statename);

public:

	void change_state(const synfig::String& statename, bool force = false);

	void refresh();

	void add_state(const Smach::state_base* state);

	Dock_Toolbox();
	virtual ~Dock_Toolbox();

	virtual void write_layout_string(std::string &params) const;
	virtual void read_layout_string(const std::string &params) const;
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
