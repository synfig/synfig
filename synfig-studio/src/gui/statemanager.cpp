/* === S Y N F I G ========================================================= */
/*!	\file statemanager.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "statemanager.h"

#include <synfig/general.h>

#include <gui/actiondatabase.h>
#include <gui/app.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

const char* action_group_name = "tool";

std::map<std::string, const char*> default_tool_accels = {
	{"normal", "s"},
	{"smooth_move", "m"},
	{"scale", "l"},
	{"rotate", "a"},
	{"mirror", "i"},
	{"circle", "e"},
	{"rectangle", "r"},
	{"star", "asterisk"},
	{"polygon", "o"},
	{"gradient", "g"},
	{"bline", "b"},
	{"draw", "p"},
	{"lasso", "c"},
	{"width", "w"},
	{"fill", "u"},
	{"eyedrop", "d"},
	{"bone", "n"},
	{"text", "t"},
	{"sketch", "k"},
	{"zoom", "z"},
};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

StateManager::StateManager()
	: state_group_(Gio::SimpleActionGroup::create())
{
	if (App::main_window)
		App::main_window->insert_action_group(action_group_name, state_group_);

	action_set_tool_ = Gio::SimpleAction::create_radio_string("set-tool", "normal");
	action_set_tool_->signal_change_state().connect(sigc::mem_fun(*this, &StateManager::on_change_state_));
	state_group_->add_action(action_set_tool_);
}

StateManager::~StateManager()
{
	App::main_window->remove_action_group(action_group_name);
}

void
StateManager::on_change_state_(const Glib::VariantBase& vb_state_name)
{
	const auto state_name = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(vb_state_name).get();
	const auto it = state_book_.find(state_name);
	if (it == state_book_.cend()) {
		synfig::warning(_("Cannot change state: inexistent state '%s'"), state_name.c_str());
		return;
	}

	Glib::ustring current_state;
	state_group_->get_action_state("set-tool", current_state);
	if (current_state == state_name) {
		return;
	}

	action_set_tool_->set_state(vb_state_name);
	signal_state_selected_.emit(it->second.state);
}

void
StateManager::change_state(const std::string& state_name)
{
	if (state_group_ && !state_name.empty()) {
		auto it = state_book_.find(state_name);
		if (it != state_book_.end()) {
			Glib::ustring current_state;
			state_group_->get_action_state("set-tool", current_state);
			if (current_state != state_name)
				state_group_->change_action_state("set-tool", Glib::Variant<Glib::ustring>::create(state_name));
		}
	}
}

void
StateManager::register_state(const Smach::state_base* state)
{
	if (!state) {
		synfig::error(_("Internal error: registering a null state."));
		return;
	}

	const String name(state->get_name());
	StateInfo state_info;
	state_info.state = state;

	// This action is to be used in menus only.
	// It is created as a regular action - instead of a stateful action -
	// to not show the radio indicator in the menu item.
	auto action = state_group_->add_action("set-tool-" + name,
										sigc::bind(
											sigc::mem_fun(*this, &StateManager::change_state),
											name
										));
	state_info.action = action;

	state_book_.emplace(name, state_info);

	const char* accel = "";
	try {
		accel = default_tool_accels.at(name);
	} catch (...) {
	}

	const std::string detailed_action_name = strprintf("%s.set-tool('%s')", action_group_name, name.c_str());
	ActionDatabase::Entry entry {detailed_action_name, state->get_local_name(), accel, state_icon_name(name), state->get_local_name()};
	App::get_action_database()->add(entry);

	signal_state_registered_.emit(state);

	// Apply default accelerators
	ActionDatabase adb;
	adb.add(entry);
	UserAcceleratorList list;
	list.restore_to_defaults(adb);
	list.load_from_file(App::get_config_file("accelerators"), false);
	list.apply(App::instance(), adb);
}

Glib::RefPtr<Gio::SimpleActionGroup>
StateManager::get_action_group() const
{
	return state_group_;
}

std::vector<std::string>
StateManager::get_state_names() const
{
	std::vector<std::string> names;
	for (const auto& pair : state_book_)
		names.push_back(pair.first);
	return names;
}
