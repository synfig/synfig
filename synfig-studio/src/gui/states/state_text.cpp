/* === S Y N F I G ========================================================= */
/*!	\file state_text.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
**  Copyright (c) 2010 Carlos López
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

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>

#include "state_text.h"
#include "state_normal.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"

#include <synfigapp/action.h>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "docks/dock_toolbox.h"
#include "docks/dialog_tooloptions.h"
#include <gtkmm/optionmenu.h>
#include "duck.h"
#include "widgets/widget_enum.h"
#include <synfigapp/main.h>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StateText studio::state_text;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateText_Context
{
	etl::handle<CanvasView> canvas_view;
	CanvasView::IsWorking is_working;

	Duckmatic::Push duckmatic_push;

	void refresh_ducks();

	bool prev_workarea_layer_status_;

	//Toolbox settings
	synfigapp::Settings& settings;

	//Toolbox display
	Gtk::Table options_table;

	Gtk::Entry		entry_id; //what to name the layer
	Gtk::Entry		entry_family;
	Widget_Vector	widget_size;
	Widget_Vector	widget_orientation;
	Gtk::CheckButton checkbutton_paragraph;

public:
	synfig::String get_id()const { return entry_id.get_text(); }
	void set_id(const synfig::String& x) { return entry_id.set_text(x); }

	bool get_paragraph_flag()const { return checkbutton_paragraph.get_active(); }
	void set_paragraph_flag(bool x) { return checkbutton_paragraph.set_active(x); }

	Vector get_size() { return widget_size.get_value(); }
	void set_size(Vector s) { return widget_size.set_value(s); }

	Vector get_orientation() { return widget_orientation.get_value(); }
	void set_orientation(Vector s) { return widget_orientation.set_value(s); }

	String get_family()const { return entry_family.get_text(); }
	void set_family(String s) { return entry_family.set_text(s); }

	void refresh_tool_options(); //to refresh the toolbox

	//events
	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	Smach::event_result event_workarea_mouse_button_down_handler(const Smach::event& x);

	//constructor destructor
	StateText_Context(CanvasView *canvas_view);
	~StateText_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view->canvas_interface();}
	WorkArea * get_work_area()const{return canvas_view->get_work_area();}

	//Modifying settings etc.
	void load_settings();
	void save_settings();
	void reset();
	void increment_id();
	bool egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& /*x*/)
	{
		if(egress_on_selection_change)
			throw &state_normal; //throw Smach::egress_exception();
		return Smach::RESULT_OK;
	}

	void make_text(const Point& point);

}; // END of class StateText_Context

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

StateText::StateText():
	Smach::state<StateText_Context>("text")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StateText_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,&StateText_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateText_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateText_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateText_Context::event_workarea_mouse_button_down_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateText_Context::event_refresh_tool_options));
}

StateText::~StateText()
{
}

void
StateText_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		String value;
		Vector v;

		//parse the arguments yargh!
		if(settings.get_value("text.id",value))
			set_id(value);
		else
			set_id("Text");

		if(settings.get_value("text.paragraph",value) && value=="1")
			set_paragraph_flag(true);
		else
			set_paragraph_flag(false);

		if(settings.get_value("text.size_x",value))
			v[0] = atof(value.c_str());
		else
			v[0] = 0.25;
		if(settings.get_value("text.size_y",value))
			v[1] = atof(value.c_str());
		else
			v[1] = 0.25;
		set_size(v);

		if(settings.get_value("text.orient_x",value))
			v[0] = atof(value.c_str());
		else
			v[0] = 0.5;
		if(settings.get_value("text.orient_y",value))
			v[1] = atof(value.c_str());
		else
			v[1] = 0.5;
		set_orientation(v);

		if(settings.get_value("text.family",value))
			set_family(value);
		else
			set_family("Sans Serif");
	}
	catch(...)
	{
		synfig::warning("State Text: Caught exception when attempting to load settings.");
	}
}

void
StateText_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		settings.set_value("text.id",get_id());
		settings.set_value("text.paragraph",get_paragraph_flag()?"1":"0");
		settings.set_value("text.size_x",strprintf("%f",(float)get_size()[0]));
		settings.set_value("text.size_y",strprintf("%f",(float)get_size()[1]));
		settings.set_value("text.orient_x",strprintf("%f",(float)get_orientation()[0]));
		settings.set_value("text.orient_y",strprintf("%f",(float)get_orientation()[1]));
		settings.set_value("text.family",get_family());
	}
	catch(...)
	{
		synfig::warning("State Text: Caught exception when attempting to save settings.");
	}
}

void
StateText_Context::reset()
{
	refresh_ducks();
}

void
StateText_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="Text";

	// If there is a number
	// already at the end of the
	// id, then remove it.
	if(id[id.size()-1]<='9' && id[id.size()-1]>='0')
	{
		// figure out how many digits it is
		for (digits = 0;
			 (int)id.size()-1 >= digits && id[id.size()-1-digits] <= '9' && id[id.size()-1-digits] >= '0';
			 digits++)
			;

		String str_number;
		str_number=String(id,id.size()-digits,id.size());
		id=String(id,0,id.size()-digits);

		number=atoi(str_number.c_str());
	}
	else
	{
		number=1;
		digits=3;
	}

	number++;

	// Add the number back onto the id
	{
		const String format(strprintf("%%0%dd",digits));
		id+=strprintf(format.c_str(),number);
	}

	// Set the ID
	set_id(id);
}

StateText_Context::StateText_Context(CanvasView *canvas_view):
	canvas_view(canvas_view),
	is_working(*canvas_view),
	duckmatic_push(get_work_area()),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	entry_id(),
	checkbutton_paragraph(_("Multiline Editor"))
{
	egress_on_selection_change=true;

	widget_size.set_digits(2);
	widget_size.set_canvas(canvas_view->get_canvas());

	widget_orientation.set_digits(2);

	options_table.attach(*manage(new Gtk::Label(_("Text Tool"))),		0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(entry_id,										0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_paragraph,							0, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(*manage(new Gtk::Label(_("Size:"))),			0, 1, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(widget_size,									1, 2, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(*manage(new Gtk::Label(_("Orientation:"))),	0, 1, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(widget_orientation,							1, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(*manage(new Gtk::Label(_("Family:"))),			0, 1, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(entry_family,									1, 2, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	load_settings();

	options_table.show_all();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// clear out the ducks
	get_work_area()->clear_ducks();

	// Refresh the work area
	get_work_area()->queue_draw();

	// Hide the tables if they are showing
	//prev_table_status=get_canvas_view()->tables_are_visible();
	//if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	//get_canvas_view()->set_sensitive_timebar(false);

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateText_Context::on_user_click));
	get_work_area()->set_cursor(Gdk::XTERM);

	App::dock_toolbox->refresh();
}

void
StateText_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Text Tool"));
	App::dialog_tool_options->set_name("text");
}

Smach::event_result
StateText_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateText_Context::~StateText_Context()
{
	save_settings();

	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);
	get_work_area()->reset_cursor();

	App::dialog_tool_options->clear();

	get_work_area()->queue_draw();

	get_canvas_view()->queue_rebuild_ducks();

	App::dock_toolbox->refresh();
}

Smach::event_result
StateText_Context::event_stop_handler(const Smach::event& /*x*/)
{
	//throw Smach::egress_exception();
	throw &state_normal;
	return Smach::RESULT_OK;
}

Smach::event_result
StateText_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

void
StateText_Context::make_text(const Point& _point)
{
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Text"));
	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

	Layer::Handle layer;

	Canvas::Handle canvas(get_canvas_view()->get_canvas());
	int depth(0);

	// we are temporarily using the layer to hold something
	layer=get_canvas_view()->get_selection_manager()->get_selected_layer();
	if(layer)
	{
		depth=layer->get_depth();
		canvas=layer->get_canvas();
	}

	synfigapp::SelectionManager::LayerList layer_selection;
	if (!getenv("SYNFIG_TOOLS_CLEAR_SELECTION"))
		layer_selection = get_canvas_view()->get_selection_manager()->get_selected_layers();

	const synfig::TransformStack& transform(get_work_area()->get_curr_transform_stack());
	const Point point(transform.unperform(_point));

	String text;
	if (get_paragraph_flag())
		App::dialog_paragraph(_("Text Paragraph"), _("Enter text here:"), text);
	else
		App::dialog_entry(_("Text Entry"), _("Enter text here:"), text);

	layer=get_canvas_interface()->add_layer_to("text",canvas,depth);
	if (!layer)
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
		group.cancel();
		return;
	}
	layer_selection.push_back(layer);

	layer->set_param("origin",point);
	get_canvas_interface()->signal_layer_param_changed()(layer,"origin");

	layer->set_param("text",text);
	get_canvas_interface()->signal_layer_param_changed()(layer,"text");

	layer->set_param("size",get_size());
	get_canvas_interface()->signal_layer_param_changed()(layer,"size");

	layer->set_param("orient",get_orientation());
	get_canvas_interface()->signal_layer_param_changed()(layer,"orient");

	layer->set_param("family",get_family());
	get_canvas_interface()->signal_layer_param_changed()(layer,"family");

	layer->set_description(get_id());
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	egress_on_selection_change=false;
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
	egress_on_selection_change=true;

	reset();
	increment_id();
}

Smach::event_result
StateText_Context::event_workarea_mouse_button_down_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	if(event.button==BUTTON_LEFT)
	{
		make_text(get_work_area()->snap_point_to_grid(event.pos));

		get_work_area()->clear_ducks();
		return Smach::RESULT_ACCEPT;
	}
	return Smach::RESULT_OK;
}

void
StateText_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();
}
