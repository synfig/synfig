/* === S Y N F I G ========================================================= */
/*!	\file state_select.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/states/state_select.h>

#include <synfig/general.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/event_keyboard.h>
#include <gui/event_layerclick.h>
#include <gui/event_mouse.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/localization.h>
#include <gui/workarea.h>

#include <synfig/angle.h>

#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef EPSILON
#define EPSILON	0.0000001
#endif

const int GAP = 3;

/* === G L O B A L S ======================================================= */

StateSelect studio::state_select;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateSelect_Context : public sigc::trackable
{
	CanvasView* canvas_view_;

	synfigapp::Settings& settings;

	etl::handle<DuckDrag_Select> duck_dragger_;

	Gtk::Grid options_grid;
	//prioritize group selection option
	Gtk::Label title_label;
	Gtk::Label prioritize_groups_label;
	Gtk::CheckButton prioritize_groups_checkbutton;
	Gtk::Box prioritize_groups_box;
	//select tool functionality
	Gtk::Label functionality_label;
	Gtk::ToggleButton move_button;
	Gtk::ToggleButton rotate_button;
	Gtk::Box functionality_box;

	enum class InnerState {
		MOVE,
		ROTATE
	};

	InnerState inner_state = InnerState::MOVE;

	void toggle_move_button();
	void toggle_rotate_button();
public:

	explicit StateSelect_Context(CanvasView* canvas_view);

	~StateSelect_Context();

	void load_settings();
	void save_settings();

	void update_state();

	//maybe from we here we can update a flag in work area... orr we can just use settings there ?
//	void set_group_priority(bool status){ prioritize_groups_checkbutton.set_active(status);}

	CanvasView* get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_undo_handler(const Smach::event& x);
	Smach::event_result event_redo_handler(const Smach::event& x);
	Smach::event_result event_refresh_ducks_handler(const Smach::event& x);
	Smach::event_result event_mouse_button_down_handler(const Smach::event& x);
	Smach::event_result event_mouse_release_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	void refresh_tool_options();
	Smach::event_result event_layer_click(const Smach::event& x);


};	// END of class StateSelect_Context

/* === M E T H O D S ======================================================= */

StateSelect::StateSelect():
	Smach::state<StateSelect_Context>("select")
{
	insert(event_def(EVENT_STOP,&StateSelect_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateSelect_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateSelect_Context::event_refresh_ducks_handler));
	insert(event_def(EVENT_UNDO,&StateSelect_Context::event_undo_handler));
	insert(event_def(EVENT_REDO,&StateSelect_Context::event_redo_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateSelect_Context::event_mouse_button_down_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateSelect_Context::event_refresh_tool_options));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,	&StateSelect_Context::event_mouse_release_handler));
	insert(event_def(EVENT_WORKAREA_LAYER_CLICKED,&StateSelect_Context::event_layer_click));
}

StateSelect::~StateSelect()
{
}

void* StateSelect::enter_state(studio::CanvasView* machine_context) const
{
	return new StateSelect_Context(machine_context);
}


void StateSelect_Context::toggle_move_button()
{
	//if move button is pressed then make all other not pressed
	if (move_button.get_active()){
		rotate_button.set_active(false);

		get_work_area()->set_duck_dragger(duck_dragger_);
		get_work_area()->set_cursor(Gdk::FLEUR);
	}
}

void StateSelect_Context::toggle_rotate_button()
{
	//if rotate button is pressed them make all others not pressed
	if (rotate_button.get_active()){
		move_button.set_active(false);

		get_work_area()->set_cursor(Gdk::EXCHANGE);
	}
}

StateSelect_Context::StateSelect_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	duck_dragger_(new DuckDrag_Select())
{
	duck_dragger_->canvas_view_=get_canvas_view();

	// Toolbox widgets
	title_label.set_label(_("Select Tool"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_hexpand();
	title_label.set_halign(Gtk::ALIGN_START);
	title_label.set_valign(Gtk::ALIGN_CENTER);

	prioritize_groups_label.set_label(_("Prioritize Group Selection"));
	prioritize_groups_label.set_hexpand();
	prioritize_groups_label.set_halign(Gtk::ALIGN_START);
	prioritize_groups_label.set_valign(Gtk::ALIGN_CENTER);
	prioritize_groups_box.pack_start(prioritize_groups_label, true, true, 0);
	prioritize_groups_box.pack_start(prioritize_groups_checkbutton, false, false, 0);

	functionality_label.set_label(_("Functionality:"));
	functionality_label.set_hexpand();
	functionality_label.set_halign(Gtk::ALIGN_START);
	functionality_label.set_valign(Gtk::ALIGN_CENTER);
	//initially move functionality is selected
	//ToDo: maybe it's better to save which functionality was prev
	//selected
	move_button.set_active(true);
	functionality_box.pack_start(functionality_label, true, true, 0);
	functionality_box.pack_start(move_button, false, false, 0);
	functionality_box.pack_start(rotate_button, false, false, 0);

	prioritize_groups_checkbutton.signal_toggled().connect(sigc::mem_fun(*this,&StateSelect_Context::save_settings));
	move_button.signal_toggled().connect(sigc::mem_fun(*this, &StateSelect_Context::toggle_move_button));
	rotate_button.signal_toggled().connect(sigc::mem_fun(*this, &StateSelect_Context::toggle_rotate_button));

	// Toolbox layout
	options_grid.attach(title_label,
		0, 0, 1, 1);
	options_grid.attach(prioritize_groups_box,
		0, 1, 2, 1);
	options_grid.attach(functionality_box,
		0, 2, 2, 1);


	options_grid.set_border_width(GAP*2);
	options_grid.set_row_spacing(GAP);
	options_grid.set_margin_bottom(0);
	options_grid.show_all();

	refresh_tool_options();

	get_work_area()->set_allow_layer_clicks(true);
	//test rotate for now dont forget to return it back
	get_work_area()->set_duck_dragger(duck_dragger_);

	App::dock_toolbox->refresh();

	load_settings();
	get_work_area()->set_cursor(Gdk::FLEUR);
}

void
StateSelect_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_grid);
	App::dialog_tool_options->set_local_name(_("Select Tool"));
	App::dialog_tool_options->set_icon("tool_select_icon");
	canvas_view_->set_duck_buttons_sensitivity(false);
}



StateSelect_Context::~StateSelect_Context()
{
	save_settings();

	get_work_area()->clear_duck_dragger();

	get_work_area()->reset_cursor();

	App::dialog_tool_options->clear();

	App::dock_toolbox->refresh();

	canvas_view_->set_duck_buttons_sensitivity(true);
}

void StateSelect_Context::load_settings()
{
	try
	{
		prioritize_groups_checkbutton.set_active(settings.get_value("select.group_selection_priority", false));
	}
	catch(...)
	{
		synfig::warning("State Select: Caught exception when attempting to load settings.");
	}
}

void StateSelect_Context::save_settings()
{
	try
	{
		settings.set_value("select.group_selection_priority", prioritize_groups_checkbutton.get_active());
	}
	catch(...)
	{
		synfig::warning("State Select: Caught exception when attempting to save settings.");
	}
}

DuckDrag_Select::DuckDrag_Select():
	DuckDrag_Combo(true)
{

}

Smach::event_result
StateSelect_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateSelect_Context::event_stop_handler(const Smach::event& /*x*/)
{
	//synfig::info("STATE SELECT: Received Stop Event");
	canvas_view_->stop();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateSelect_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	//synfig::info("STATE SELECT: Received Refresh Event");
	canvas_view_->rebuild_tables();
	canvas_view_->get_work_area()->queue_render();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateSelect_Context::event_undo_handler(const Smach::event& /*x*/)
{
	//synfig::info("STATE SELECT: Received Undo Event");
	canvas_view_->get_instance()->undo();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateSelect_Context::event_redo_handler(const Smach::event& /*x*/)
{
	//synfig::info("STATE SELECT: Received Redo Event");
	canvas_view_->get_instance()->redo();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateSelect_Context::event_refresh_ducks_handler(const Smach::event& /*x*/)
{
	//synfig::info("STATE NORMAL: Received Refresh Ducks");
	canvas_view_->queue_rebuild_ducks();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateSelect_Context::event_mouse_button_down_handler(const Smach::event& x)
{
	//synfig::info("STATE SELECT: Received mouse button down Event");

	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_RIGHT:
		canvas_view_->popup_main_menu();
		return Smach::RESULT_ACCEPT;
	default:
		return Smach::RESULT_OK;
	}
}

Smach::event_result
StateSelect_Context::event_mouse_release_handler(const Smach::event& x){
	return Smach::RESULT_OK;
}

Smach::event_result
StateSelect_Context::event_layer_click(const Smach::event& x)
{
	const EventLayerClick& event(*reinterpret_cast<const EventLayerClick*>(&x));

	switch(event.button)
	{
	case BUTTON_LEFT:
		if(!(event.modifier&Gdk::CONTROL_MASK))
			canvas_view_->get_selection_manager()->clear_selected_layers();
		if(event.layer)
		{
			std::list<Layer::Handle> layer_list(canvas_view_->get_selection_manager()->get_selected_layers());
			std::set<Layer::Handle> layers(layer_list.begin(),layer_list.end());

			if(!layers.count(event.layer))
			{
				canvas_view_->get_selection_manager()->set_selected_layer(event.layer);
			}

		}
		return Smach::RESULT_ACCEPT;
	case BUTTON_RIGHT:
		canvas_view_->popup_layer_menu(event.layer);
		return Smach::RESULT_ACCEPT;
	default:
		return Smach::RESULT_OK;
	}
}
