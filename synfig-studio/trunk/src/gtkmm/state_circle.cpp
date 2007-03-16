/* === S Y N F I G ========================================================= */
/*!	\file state_circle.cpp
**	\brief Template File
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

#include <synfig/valuenode_dynamiclist.h>
#include <synfigapp/action_system.h>

#include "state_circle.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"

#include <synfigapp/action.h>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "toolbox.h"
#include "dialog_tooloptions.h"
#include <gtkmm/optionmenu.h>
#include "duck.h"
#include "widget_enum.h"
#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */
enum CircleFalloff
{
	CIRCLE_SQUARED	=0,
	CIRCLE_INTERPOLATION_LINEAR	=1,
	CIRCLE_COSINE	=2,
	CIRCLE_SIGMOND	=3,
	CIRCLE_SQRT		=4,
	CIRCLE_NUM_FALLOFF
};

/* === G L O B A L S ======================================================= */

StateCircle studio::state_circle;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateCircle_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	Duckmatic::Push duckmatic_push;

	Point point_holder;

	etl::handle<Duck> point2_duck;

	void refresh_ducks();

	bool prev_workarea_layer_status_;

	//Toolbox settings
	synfigapp::Settings& settings;

	//Toolbox display
	Gtk::Table options_table;

	Gtk::Entry		entry_id; //what to name the layer

	Widget_Enum		enum_falloff;
	Widget_Enum		enum_blend;

	Gtk::Adjustment	adj_feather;
	Gtk::SpinButton	spin_feather;

	Gtk::CheckButton check_invert;

public:

	synfig::String get_id()const { return entry_id.get_text(); }
	void set_id(const synfig::String& x) { return entry_id.set_text(x); }

	int get_falloff()const { return enum_falloff.get_value(); }
	void set_falloff(int x) { return enum_falloff.set_value(x); }

	int get_blend()const { return enum_blend.get_value(); }
	void set_blend(int x) { return enum_blend.set_value(x); }

	Real get_feather()const { return adj_feather.get_value(); }
	void set_feather(Real f) { adj_feather.set_value(f); }

	bool get_invert()const { return check_invert.get_active(); }
	void set_invert(bool i) { check_invert.set_active(i); }

	void refresh_tool_options(); //to refresh the toolbox

	//events
	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	//constructor destructor
	StateCircle_Context(CanvasView* canvas_view);
	~StateCircle_Context();

	//Canvas interaction
	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	//Modifying settings etc.
	void load_settings();
	void save_settings();
	void reset();
	void increment_id();
	bool no_egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& x)
	{
		if(!no_egress_on_selection_change)
			throw Smach::egress_exception();
		return Smach::RESULT_OK;
	}

	void make_circle(const Point& p1, const Point& p2);

};	// END of class StateGradient_Context

/* === M E T H O D S ======================================================= */

StateCircle::StateCircle():
	Smach::state<StateCircle_Context>("circle")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StateCircle_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,&StateCircle_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateCircle_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateCircle_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateCircle_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateCircle_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateCircle_Context::event_mouse_click_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateCircle_Context::event_refresh_tool_options));
}

StateCircle::~StateCircle()
{
}

void
StateCircle_Context::load_settings()
{
	String value;

	//parse the arguments yargh!
	if(settings.get_value("circle.id",value))
		set_id(value);
	else
		set_id("Circle");

	if(settings.get_value("circle.fallofftype",value) && value != "")
		set_falloff(atoi(value.c_str()));
	else
		set_falloff(2);

	if(settings.get_value("circle.blend",value) && value != "")
		set_blend(atoi(value.c_str()));
	else
		set_blend(0);//(int)Color::BLEND_COMPOSITE); //0 should be blend composites value

	if(settings.get_value("circle.feather",value))
		set_feather(atof(value.c_str()));
	else
		set_feather(0);

	if(settings.get_value("circle.invert",value) && value != "0")
		set_invert(true);
	else
		set_invert(false);
}

void
StateCircle_Context::save_settings()
{
	settings.set_value("circle.id",get_id());
	settings.set_value("circle.fallofftype",strprintf("%d",get_falloff()));
	settings.set_value("circle.blend",strprintf("%d",get_blend()));
	settings.set_value("circle.feather",strprintf("%f",(float)get_feather()));
	settings.set_value("circle.invert",get_invert()?"1":"0");
}

void
StateCircle_Context::reset()
{
	refresh_ducks();
}

void
StateCircle_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="Circle";

	// If there is a number
	// already at the end of the
	// id, then remove it.
	if(id[id.size()-1]<='9' && id[id.size()-1]>='0')
	{
		// figure out how many digits it is
		for(digits=0;(int)id.size()-1>=digits && id[id.size()-1-digits]<='9' && id[id.size()-1-digits]>='0';digits++)while(false);

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

StateCircle_Context::StateCircle_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	duckmatic_push(get_work_area()),
	prev_workarea_layer_status_(get_work_area()->allow_layer_clicks),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	entry_id(),
	adj_feather(0,0,1,0.01,0.1),
	spin_feather(adj_feather,0.1,3),
	check_invert(_("Invert"))
{
	no_egress_on_selection_change=false;
	// Set up the tool options dialog
	//options_table.attach(*manage(new Gtk::Label(_("Circle Tool"))), 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(entry_id, 0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	enum_falloff.set_param_desc(ParamDesc("falloff")
		.set_local_name(_("Falloff"))
		.set_description(_("Determines the falloff function for the feather"))
		.set_hint("enum")
		.add_enum_value(CIRCLE_INTERPOLATION_LINEAR,"linear",_("Linear"))
		.add_enum_value(CIRCLE_SQUARED,"squared",_("Squared"))
		.add_enum_value(CIRCLE_SQRT,"sqrt",_("Square Root"))
		.add_enum_value(CIRCLE_SIGMOND,"sigmond",_("Sigmond"))
		.add_enum_value(CIRCLE_COSINE,"cosine",_("Cosine")));

	enum_blend.set_param_desc(ParamDesc(Color::BLEND_COMPOSITE,"blend_method")
		.set_local_name(_("Blend Method"))
		.set_description(_("Defines the blend method to be used for circles")));

	load_settings();

	//feather stuff
	options_table.attach(*manage(new Gtk::Label(_("Feather:"))), 0, 1, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_feather, 1, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(enum_falloff, 0, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(enum_blend, 0, 2, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	//invert flag
	options_table.attach(check_invert, 0, 2, 6, 7, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.show_all();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// Turn off layer clicking
	get_work_area()->allow_layer_clicks=false;

	// clear out the ducks
	get_work_area()->clear_ducks();

	// Refresh the work area
	get_work_area()->queue_draw();

	// Hide the tables if they are showing
	//prev_table_status=get_canvas_view()->tables_are_visible();
	//if(prev_table_status)get_canvas_view()->hide_tables();

	// Hide the time bar
	//get_canvas_view()->hide_timebar();

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateCircle_Context::on_user_click));
	get_canvas_view()->work_area->set_cursor(Gdk::CROSSHAIR);

	App::toolbox->refresh();
}

void
StateCircle_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Circle Tool"));
	App::dialog_tool_options->set_name("circle");
}

Smach::event_result
StateCircle_Context::event_refresh_tool_options(const Smach::event& x)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateCircle_Context::~StateCircle_Context()
{
	save_settings();

	// Restore layer clicking
	get_work_area()->allow_layer_clicks=prev_workarea_layer_status_;
	get_canvas_view()->work_area->reset_cursor();

	App::dialog_tool_options->clear();

	// Show the time bar
	if(get_canvas_view()->get_canvas()->rend_desc().get_time_start()!=get_canvas_view()->get_canvas()->rend_desc().get_time_end())
		get_canvas_view()->show_timebar();

	// Bring back the tables if they were out before
	//if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	App::toolbox->refresh();
}

Smach::event_result
StateCircle_Context::event_stop_handler(const Smach::event& x)
{
	throw Smach::egress_exception();
}

Smach::event_result
StateCircle_Context::event_refresh_handler(const Smach::event& x)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

void
StateCircle_Context::make_circle(const Point& _p1, const Point& _p2)
{
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Circle"));
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

	const synfig::TransformStack& transform(get_canvas_view()->get_curr_transform_stack());
	const Point p1(transform.unperform(_p1));
	const Point p2(transform.unperform(_p2));

	if(get_falloff() >= 0 && get_falloff() < CIRCLE_NUM_FALLOFF)
	{

		layer=get_canvas_interface()->add_layer_to("circle",canvas,depth);

		layer->set_param("pos",p1);
		get_canvas_interface()->signal_layer_param_changed()(layer,"pos");

		layer->set_param("radius",(p2-p1).mag());
		get_canvas_interface()->signal_layer_param_changed()(layer,"radius");

		layer->set_param("falloff",get_falloff());
		get_canvas_interface()->signal_layer_param_changed()(layer,"falloff");

		layer->set_param("feather",get_feather());
		get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

		layer->set_param("invert",get_invert());
		get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

		layer->set_param("blend_method",get_blend());
		get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

		layer->set_description(get_id());
		get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

		no_egress_on_selection_change=true;
		get_canvas_interface()->get_selection_manager()->clear_selected_layers();
		get_canvas_interface()->get_selection_manager()->set_selected_layer(layer);
		no_egress_on_selection_change=false;
	}

	reset();
	increment_id();
}

Smach::event_result
StateCircle_Context::event_mouse_click_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_DOWN && event.button==BUTTON_LEFT)
	{
		point_holder=get_work_area()->snap_point_to_grid(event.pos);
		etl::handle<Duck> duck=new Duck();
		duck->set_point(point_holder);
		duck->set_name("p1");
		duck->set_type(Duck::TYPE_POSITION);
		duck->set_editable(false);
		get_work_area()->add_duck(duck);

		point2_duck=new Duck();
		point2_duck->set_point(Vector(0,0));
		point2_duck->set_name("radius");
		point2_duck->set_origin(duck);
		point2_duck->set_radius(true);
		point2_duck->set_scalar(-1);
		point2_duck->set_type(Duck::TYPE_RADIUS);
		get_work_area()->add_duck(point2_duck);

		return Smach::RESULT_ACCEPT;
	}

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_DRAG && event.button==BUTTON_LEFT)
	{
		point2_duck->set_point(point_holder-get_work_area()->snap_point_to_grid(event.pos));
		get_work_area()->queue_draw();
		return Smach::RESULT_ACCEPT;
	}

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_UP && event.button==BUTTON_LEFT)
	{
		make_circle(point_holder, get_work_area()->snap_point_to_grid(event.pos));
		get_work_area()->clear_ducks();
		return Smach::RESULT_ACCEPT;
	}

	return Smach::RESULT_OK;
}


void
StateCircle_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();
}
