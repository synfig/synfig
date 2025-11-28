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

#include <gtkmm/accelmap.h>
#include <gtkmm/paned.h>
#include <gtkmm/toolpalette.h>

#include <gui/actionmanagers/actionmanager.h>
#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/instance.h>
#include <gui/localization.h>
#include <gui/widgets/widget_defaults.h>

#include <synfig/general.h>

#include <synfigapp/main.h>

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

	Gtk::ToolPalette *palette = manage(new Gtk::ToolPalette());
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
		sigc::hide(
			sigc::mem_fun(*this,&studio::Dock_Toolbox::update_tools) ));

	std::vector<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("text/plain") );
	listTargets.push_back( Gtk::TargetEntry("image") );
//	listTargets.push_back( Gtk::TargetEntry("image/x-sif") );

	drag_dest_set(listTargets);
	signal_drag_data_received().connect( sigc::mem_fun(*this, &studio::Dock_Toolbox::on_drop_drag_data_received) );

	App::signal_present_all().connect(sigc::mem_fun0(*this,&Dock_Toolbox::present));
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

	try {
		for (const auto& item : state_button_map) {
			if (item.first == statename && !item.second.first->get_active()) {
				item.second.first->set_active(true);
				break;
			}
		}
	} catch (...) {
		changing_state_ = false;
		throw;
	}

	changing_state_ = false;
}

void
Dock_Toolbox::change_state(const synfig::String& statename, bool force)
{
	studio::CanvasView::Handle canvas_view(studio::App::get_selected_canvas_view());
	if(canvas_view)
	{
		if(!force && statename==canvas_view->get_smach().get_state_name())
			return;

		auto iter = state_button_map.find(statename);
		if (iter != state_button_map.end()) {
			change_state_(static_cast<const Smach::state_base*>(iter->second.second));
		} else {
			synfig::error("Unknown state \"%s\"",statename.c_str());
		}
	}
}

void
Dock_Toolbox::change_state_(const Smach::state_base *state)
{

	try
	{
		studio::CanvasView::Handle canvas_view(studio::App::get_selected_canvas_view());
		if(canvas_view)
			canvas_view->get_smach().enter(state);
		else
			refresh();
	}
	catch(...)
	{
		throw;
	}
}


/*! \fn Dock_Toolbox::add_state(const Smach::state_base *state)
 *  \brief Add and connect a toggle button to the toolbox defined by a state
 *  \param state a const pointer to Smach::state_base
*/
void
Dock_Toolbox::add_state(const Smach::state_base* state)
{
	assert(state);

	String name=state->get_name();

	const ActionManager::Entry& entry = App::get_action_manager()->get("win.set-tool-" + name);

	Gtk::RadioToolButton *tool_button = manage(new Gtk::RadioToolButton());
	tool_button->set_label(entry.label_);
	// Sadly not all tool icons (or tool names) follow a convention
	const std::string symbolic_suffix = ""; // App::use-symbolic-icons ? "-symbolic" : "";
	tool_button->set_icon_name(entry.icon_ + symbolic_suffix);
	tool_button->set_group(radio_tool_button_group);

	// Keeps updating the tooltip if user changes the shortcut at runtime
	tool_button->property_has_tooltip() = true;
	tool_button->signal_query_tooltip().connect([entry](int,int,bool,const Glib::RefPtr<Gtk::Tooltip>& tooltip) -> bool
	{
		tooltip->set_text(entry.get_tooltip(App::instance()));
		return true;
	});
//	tool_button->set_tooltip_text(get_tooltip(name));
	tool_button->show();

	tool_item_group->insert(*tool_button);
	tool_item_group->show_all();

	state_button_map[name] = std::make_pair(tool_button, state);

	tool_button->signal_clicked().connect([name]() {
		App::instance()->main_window->activate_action("set-tool-" + name);
	});

	refresh();
}


void
Dock_Toolbox::update_tools()
{
	etl::handle<Instance> instance = App::get_selected_instance();
	CanvasView::Handle canvas_view = App::get_selected_canvas_view();

	// These next several lines just adjust the tool buttons
	// so that they are only clickable when they should be.
	// Disable buttons if there isn't any open document instance
	bool sensitive = instance && canvas_view;
	for (const auto& item : state_button_map)
		item.second.first->set_sensitive(sensitive);

	if (canvas_view && canvas_view->get_smach().get_state_name())
		set_active_state(canvas_view->get_smach().get_state_name());
	else
		set_active_state("none");
}


void
Dock_Toolbox::refresh()
{
	update_tools();
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
