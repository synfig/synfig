/* === S Y N F I G ========================================================= */
/*!	\file statemanager.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "statemanager.h"

#include <gtkmm/action.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/stock.h>

#include <gui/app.h>
#include <gui/docks/dock_toolbox.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

StateManager::StateManager():
	state_group(Gtk::ActionGroup::create("action_group_state_manager")),
	merge_id(App::ui_manager()->new_merge_id())
{
	App::ui_manager()->insert_action_group(get_action_group());
}

StateManager::~StateManager()
{
	App::ui_manager()->remove_ui(merge_id);

	for(;!merge_id_list.empty();merge_id_list.pop_back())
		App::ui_manager()->remove_ui(merge_id_list.back());
}

void
StateManager::change_state_(const Smach::state_base *state)
{
	App::dock_toolbox->change_state_(state);
}

void
StateManager::add_state(const Smach::state_base *state)
{
	String name(state->get_name());

	Gtk::StockItem stock_item;
	Gtk::Stock::lookup(Gtk::StockID("synfig-"+name),stock_item);

	Glib::RefPtr<Gtk::Action> action(
		Gtk::Action::create(
			"state-"+name,
			stock_item.get_stock_id(),
			stock_item.get_label(),
			stock_item.get_label()
		)
	);
	/*action->set_sensitive(false);*/
	state_group->add(action);

	action->signal_activate().connect(
		sigc::bind(
			sigc::mem_fun(*this,&studio::StateManager::change_state_),
			state
		)
	);

	String uid_def;
	uid_def = "<ui><popup action='menu-main'><menu action='menu-toolbox'><menuitem action='state-"+name+"' /></menu></popup></ui>";
	merge_id_list.push_back(App::ui_manager()->add_ui_from_string(uid_def));
	uid_def = "<ui><menubar action='menubar-main'><menu action='menu-toolbox'><menuitem action='state-"+name+"' /></menu></menubar></ui>";
	merge_id_list.push_back(App::ui_manager()->add_ui_from_string(uid_def));

	App::ui_manager()->ensure_update();

	App::dock_toolbox->add_state(state);
}

Glib::RefPtr<Gtk::ActionGroup>
StateManager::get_action_group()
{
	return state_group;
}
