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

#include <map>
#include <string>
#include <vector>

#include <giomm/simpleactiongroup.h>

#include <gui/smach.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateManager
{
private:
	Glib::RefPtr<Gio::SimpleActionGroup> state_group_;
	Glib::RefPtr<Gio::SimpleAction> action_set_tool_;

	struct StateInfo {
		const Smach::state_base* state;
		Glib::RefPtr<Gio::SimpleAction> action;
	};
	std::map<std::string, StateInfo> state_book_;

	void on_change_state_(const Glib::VariantBase& vb_state_name);
	void change_state(const std::string& state);

	sigc::signal<void, const Smach::state_base*> signal_state_registered_;
	sigc::signal<void, const Smach::state_base*> signal_state_selected_;

public:
	StateManager();
	~StateManager();

	void register_state(const Smach::state_base* state);

	Glib::RefPtr<Gio::SimpleActionGroup> get_action_group() const;
	std::vector<std::string> get_state_names() const;

	sigc::signal<void, const Smach::state_base*>& signal_state_registered() { return signal_state_registered_; };

	sigc::signal<void, const Smach::state_base*>& signal_state_selected() { return signal_state_selected_; };
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
