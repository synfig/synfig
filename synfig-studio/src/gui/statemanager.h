/* === S Y N F I G ========================================================= */
/*!	\file statemanager.h
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

#ifndef SYNFIG_STUDIO_STATEMANAGER_H
#define SYNFIG_STUDIO_STATEMANAGER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/radioaction.h>

#include <gui/smach.h>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

typedef unsigned int guint;

namespace studio {

class StateManager
{
private:
	Glib::RefPtr<Gtk::ActionGroup> state_group;
	Gtk::RadioAction::Group radio_action_group;

	guint merge_id;
	std::vector<guint> merge_id_list;

	void change_state_(const Glib::RefPtr<Gtk::RadioAction>& current, const Smach::state_base* state);
	void change_state(const Smach::state_base* state);

	sigc::signal<void, const Smach::state_base*> signal_state_registered_;
	sigc::signal<void, const Smach::state_base*> signal_state_selected_;

public:
	StateManager();

	~StateManager();

	void register_state(const Smach::state_base* state);

	Glib::RefPtr<Gtk::ActionGroup> get_action_group();

	sigc::signal<void, const Smach::state_base*>& signal_state_registered() { return signal_state_registered_; };

	sigc::signal<void, const Smach::state_base*>& signal_state_selected() { return signal_state_selected_; };
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
