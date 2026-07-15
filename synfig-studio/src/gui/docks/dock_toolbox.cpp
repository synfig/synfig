/* === S Y N F I G ========================================================= */
/*!	\file dock_toolbox.cpp
**	\brief writeme
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2008 Paul Wise
**	Copyright (c) 2009 Nikita Kitaev
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/docks/dock_toolbox.h>

#include <gtkmm/paned.h>
#include <gtkmm/toolpalette.h>

#include <synfig/general.h>

#include <synfigapp/main.h>

#include <gui/actiondatabase.h>
#include <gui/actionwidgethelper.h>
#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/instance.h>
#include <gui/localization.h>
#include <gui/statemanager.h>
#include <gui/widgets/widget_defaults.h>

#endif

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Dock_Toolbox::Dock_Toolbox():
	Dockable("toolbox",_("Toolbox"),"about_icon")
{
	set_use_scrolled(false);
	set_size_request(-1,-1);

	tool_item_group = manage(new class Gtk::ToolItemGroup());
	gtk_tool_item_group_set_label(tool_item_group->gobj(), nullptr);

	Gtk::ToolPalette* palette = manage(new Gtk::ToolPalette());
	palette->add(*tool_item_group);
	palette->set_expand(*tool_item_group);
	palette->set_exclusive(*tool_item_group, true);
	palette->set_icon_size(Gtk::IconSize::from_name("synfig-small_icon_16x16"));
	// let the palette propagate the scroll events
	palette->add_events(Gdk::SCROLL_MASK);
	palette->show();

	Gtk::ScrolledWindow *scrolled_window = manage(new Gtk::ScrolledWindow());
	scrolled_window->add(*palette);
	scrolled_window->set_border_width(2);
	scrolled_window->show();

	Widget_Defaults* widget_defaults(manage(new Widget_Defaults()));

	tool_box_paned = manage(new Gtk::Paned(Gtk::ORIENTATION_VERTICAL));
	tool_box_paned->pack1(*scrolled_window, Gtk::PACK_EXPAND_WIDGET|Gtk::PACK_SHRINK, false);
	tool_box_paned->pack2(*widget_defaults, Gtk::PACK_EXPAND_WIDGET|Gtk::PACK_SHRINK, false);
	tool_box_paned->set_position(200);
	tool_box_paned->show_all();
	add(*tool_box_paned);

	App::signal_canvas_view_focus().connect(
			sigc::mem_fun(*this, &studio::Dock_Toolbox::on_canvas_view_focus_changed) );

	std::vector<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("text/plain") );
	listTargets.push_back( Gtk::TargetEntry("image") );
//	listTargets.push_back( Gtk::TargetEntry("image/x-sif") );

	drag_dest_set(listTargets);
	signal_drag_data_received().connect( sigc::mem_fun(*this, &studio::Dock_Toolbox::on_drop_drag_data_received) );

	App::signal_present_all().connect(sigc::mem_fun0(*this,&Dock_Toolbox::present));

	App::get_state_manager()->signal_state_registered().connect(sigc::mem_fun(*this, &Dock_Toolbox::add_state));
	App::get_state_manager()->signal_state_selected().connect(sigc::mem_fun(*this, &Dock_Toolbox::on_state_changed));

	const std::string action_group_name = "tool";
	App::main_window->insert_action_group(action_group_name, App::get_state_manager()->get_action_group());
}

Dock_Toolbox::~Dock_Toolbox()
{
	hide();
	//studio::App::cb.task(_("Toolbox: I was nailed!"));
	//studio::App::quit();

	if (studio::App::dock_toolbox == this)
		studio::App::dock_toolbox = nullptr;
}

void Dock_Toolbox::write_layout_string(std::string& params) const
{
	char num_str[6];
	snprintf(num_str, 5, "%d", tool_box_paned->get_position());
	params += std::string(num_str);
}

void Dock_Toolbox::read_layout_string(const std::string& params) const
{
	try {
		int pos = std::stoi(params.c_str());
		tool_box_paned->set_position(pos);
	} catch (...) {
		// ignores invalid value and let it use the default one
	}
}

void
Dock_Toolbox::set_active_state(const synfig::String& statename)
{
	synfigapp::Main::set_state(statename);
}

void
Dock_Toolbox::change_state(const synfig::String& statename, bool force)
{
	studio::CanvasView::Handle canvas_view(studio::App::get_selected_canvas_view());
	if(canvas_view)
	{
		if(!force && statename==canvas_view->get_smach().get_state_name())
		{
			return;
		}

		if(state_button_map.count(statename))
		{
			state_button_map[statename]->activate();
		}
		else
		{
			synfig::error("Unknown state \"%s\"",statename.c_str());
		}
	}
}

void
Dock_Toolbox::on_state_changed(const Smach::state_base* state)
{
	CanvasView::Handle canvas_view(App::get_selected_canvas_view());
	if (canvas_view)
		canvas_view->change_state(state);
	set_active_state(state->get_name());
	auto it = state_button_map.find(state->get_name());
	if (it != state_button_map.end())
		it->second->property_active() = true;
}

/*! \fn Dock_Toolbox::add_state(const Smach::state_base* state)
 *  \brief Add and connect a toggle button to the toolbox defined by a state
 *  \param state a const pointer to Smach::state_base
*/
void
Dock_Toolbox::add_state(const Smach::state_base* state)
{
	assert(state);

	const String tool_name = state->get_name();

	Gtk::RadioToolButton* radio_button = manage(new Gtk::RadioToolButton());
	for (int i = 0; i < tool_item_group->get_n_items(); ++i) {
		if (auto radio_tool_button = dynamic_cast<Gtk::RadioToolButton*>(tool_item_group->get_nth_item(i))) {
			radio_button->join_group(*radio_tool_button);
			break;
		}
	}

	const std::string action_group_name = "tool";
	const std::string action_name = strprintf("set-tool('%s')", tool_name.c_str());
	const std::string detailed_action_name = action_group_name + "." + action_name;

	try {
		const ActionDatabase::Entry action_entry = App::get_action_database()->get(detailed_action_name);
		if (!action_entry.icon_.empty() && action_entry.icon_ != "image-missing") {
			const std::string symbolic_suffix = ""; // App::use-symbolic-icons ? "-symbolic" : "";
			radio_button->set_icon_name(action_entry.icon_ + symbolic_suffix);
		}
		ActionWidgetHelper::set_widget_tooltip(*radio_button, detailed_action_name, action_entry.tooltip_);
	} catch (...) {
		synfig::error(_("Couldn't find action: %s"), action_name.c_str());
	}

	radio_button->signal_toggled().connect(sigc::track_obj([radio_button, tool_name]() {
		if (!radio_button->get_active())
			return;
		auto action_group = App::get_state_manager()->get_action_group();
		Glib::ustring current_state;
		action_group->get_action_state("set-tool", current_state);
		if (current_state != tool_name)
			action_group->change_action_state("set-tool", Glib::Variant<Glib::ustring>::create(tool_name));

	}, App::get_state_manager()));

	radio_button->show();
	tool_item_group->insert(*radio_button);

	state_button_map[tool_name] = radio_button;
}

void
Dock_Toolbox::on_canvas_view_focus_changed(etl::loose_handle<CanvasView> canvas_view)
{

	// Disable buttons if there isn't any open document instance
	const bool sensitive = canvas_view.get();
	const std::string action_group_name = "tool";
	Glib::RefPtr<Gio::ActionGroup> action_group;
	// state_action_group->set_sensitive(sensitive);
	if (!sensitive) {
		App::main_window->remove_action_group(action_group_name);
	} else {
		if (!App::main_window->get_action_group(action_group_name)) {
			App::main_window->insert_action_group(action_group_name, App::get_state_manager()->get_action_group());
		}
		action_group = App::get_state_manager()->get_action_group();
	}

	if (!action_group)
		return;

	const char* canvasview_state_name = canvas_view ? canvas_view->get_smach().get_state_name() : nullptr;
	if (canvasview_state_name) {
		const auto state_it = state_button_map.find(canvasview_state_name);
		if (state_it != state_button_map.cend()) {
			// action_group->change_action_state("set-tool", Glib::Variant<Glib::ustring>::create(canvasview_state_name));
			set_active_state(canvasview_state_name);
			auto radio_button = state_it->second;
			if (radio_button) {
				if (!radio_button->get_active())
					radio_button->set_active(true);
			}
		} else {
			set_active_state("none");
		}
	} else {
		set_active_state("none");
	}
}

void
Dock_Toolbox::update_tools(etl::loose_handle<CanvasView> canvas_view)
{

}


void
Dock_Toolbox::refresh()
{
	CanvasView::Handle canvas_view = App::get_selected_canvas_view();
	update_tools(canvas_view);
}


void
Dock_Toolbox::on_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int /*x*/, int /*y*/, const Gtk::SelectionData& selection_data_, guint /*info*/, guint time)
{
	// We will make this true once we have a solid drop
	bool success(false);

	if ((selection_data_.get_length() >= 0) && (selection_data_.get_format() == 8))
	{
		synfig::String selection_data((gchar *)(selection_data_.get_data()));

		std::stringstream stream(selection_data);

		while(stream)
		{
			synfig::String line;
			getline(stream, line);

			line = trim(line);

			// If we don't have a filename, move on.
			if (line.empty())
				continue;

			filesystem::Path filename = filesystem::Path::from_uri(line);

			synfig::info("Attempting to open %s", filename.u8_str());
			if(App::open(filename))
				success=true;
			else
				synfig::error("Drop failed: Unable to open %s", filename.u8_str());
		}
	}
	else
		synfig::error("Drop failed: bad selection data");

	// Finish the drag
	context->drag_finish(success, false, time);
}
