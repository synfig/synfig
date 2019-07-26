/* === S Y N F I G ========================================================= */
/*!	\file dock_toolbox.h
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

#ifndef __SYNFIG_GTKMM_DOCK_TOOLBOX_H
#define __SYNFIG_GTKMM_DOCK_TOOLBOX_H

/* === H E A D E R S ======================================================= */

#include "docks/dockable.h"

#include <gtkmm/toolbar.h>
#include <gtkmm/tooltip.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/toggletoolbutton.h>
#include <gtkmm/toolpalette.h>
#include <gtkmm/toolitemgroup.h>
#include <gtkmm/alignment.h>
#include <gtkmm/table.h>
#include <gtkmm/box.h>
#include <synfig/string.h>
#include "smach.h"
#include <map>
#include "dialogsettings.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateManager;

class Dock_Toolbox : public Dockable
{
	friend class studio::StateManager;

	Gtk::ToolItemGroup *tool_item_group;
	Gtk::VBox *tool_box;

	std::map<synfig::String,Gtk::ToggleToolButton *> state_button_map;

	bool changing_state_;

	void on_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);

	void change_state_(const Smach::state_base *state);

public:

	void change_state(const synfig::String& statename, bool force = false);

	void update_tools();

	void refresh() { update_tools(); }

	void set_active_state(const synfig::String& statename);

	void add_state(const Smach::state_base *state);

	Dock_Toolbox();
	virtual ~Dock_Toolbox();

};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
