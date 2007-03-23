/* === S Y N F I G ========================================================= */
/*!	\file state_gradient.cpp
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

#include "state_gradient.h"
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

enum
{
	GRADIENT_INTERPOLATION_LINEAR=0,
	GRADIENT_RADIAL=1,
	GRADIENT_CONICAL=2,
	GRADIENT_SPIRAL=3
};

/* === G L O B A L S ======================================================= */

StateGradient studio::state_gradient;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateGradient_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	Duckmatic::Push duckmatic_push;

	synfigapp::Settings& settings;

	Point point_holder;

	etl::handle<Duck> point2_duck;

	void refresh_ducks();

	bool prev_workarea_layer_status_;

	Gtk::Table options_table;
	Gtk::Entry entry_id;
	Widget_Enum enum_type;
	Widget_Enum	enum_blend;

public:
	synfig::String get_id()const { return entry_id.get_text(); }
	void set_id(const synfig::String& x) { return entry_id.set_text(x); }

	int get_type()const { return enum_type.get_value(); }
	void set_type(int x) { return enum_type.set_value(x); }

	int get_blend()const { return enum_blend.get_value(); }
	void set_blend(int x) { return enum_blend.set_value(x); }

	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	void refresh_tool_options();

	StateGradient_Context(CanvasView* canvas_view);

	~StateGradient_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	//void on_user_click(synfig::Point point);
	void load_settings();
	void save_settings();
	void reset();
	void increment_id();

	void make_gradient(const Point& p1, const Point& p2);
	bool no_egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& x)
	{
		if(!no_egress_on_selection_change)
			throw Smach::egress_exception();
		return Smach::RESULT_OK;
	}

};	// END of class StateGradient_Context

/* === M E T H O D S ======================================================= */

StateGradient::StateGradient():
	Smach::state<StateGradient_Context>("gradient")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StateGradient_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,&StateGradient_Context::event_stop_handler));
	insert(event_def(EVENT_TABLES_SHOW,&StateGradient_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateGradient_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateGradient_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateGradient_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateGradient_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateGradient_Context::event_mouse_click_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateGradient_Context::event_refresh_tool_options));
}

StateGradient::~StateGradient()
{
}

void
StateGradient_Context::load_settings()
{
	String value;

	if(settings.get_value("gradient.id",value))
		set_id(value);
	else
		set_id("Gradient");

	if(settings.get_value("gradient.type",value))
		set_type(atoi(value.c_str()));
	else
		set_type(GRADIENT_INTERPOLATION_LINEAR);

	if(settings.get_value("gradient.blend",value))
		set_blend(atoi(value.c_str()));
	else
		set_blend(Color::BLEND_COMPOSITE);
}

void
StateGradient_Context::save_settings()
{
	settings.set_value("gradient.id",get_id().c_str());
	settings.set_value("gradient.type",strprintf("%d",get_type()));
	settings.set_value("gradient.blend",strprintf("%d",get_blend()));
}

void
StateGradient_Context::reset()
{
	refresh_ducks();
}

void
StateGradient_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="Gradient";

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

StateGradient_Context::StateGradient_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	duckmatic_push(get_work_area()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	prev_workarea_layer_status_(get_work_area()->allow_layer_clicks),
	entry_id()
{
	no_egress_on_selection_change=false;
	// Set up the tool options dialog
	///options_table.attach(*manage(new Gtk::Label(_("Gradient Tool"))), 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(entry_id, 0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	enum_type.set_param_desc(ParamDesc("type")
		.set_local_name(_("Gradient Type"))
		.set_description(_("Determines the type of Gradient used"))
		.set_hint("enum")
		.add_enum_value(GRADIENT_INTERPOLATION_LINEAR,"linear",_("Linear"))
		.add_enum_value(GRADIENT_RADIAL,"radial",_("Radial"))
		.add_enum_value(GRADIENT_CONICAL,"conical",_("Conical"))
		.add_enum_value(GRADIENT_SPIRAL,"spiral",_("Spiral")));

	enum_blend.set_param_desc(ParamDesc(Color::BLEND_COMPOSITE,"blend_method")
		.set_local_name(_("Blend Method"))
		.set_description(_("The blend method the gradient will use")));

	load_settings();

	options_table.attach(enum_type, 0, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(enum_blend, 0, 2, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.show_all();
	refresh_tool_options();
	App::dialog_tool_options->present();


	// Turn off layer clicking
	get_work_area()->allow_layer_clicks=false;

	get_canvas_view()->work_area->set_cursor(Gdk::CROSSHAIR);

	// clear out the ducks
	get_work_area()->clear_ducks();

	// Refresh the work area
	get_work_area()->queue_draw();

	get_work_area()->refresh_cursor();

	// Hide the tables if they are showing
	get_canvas_view()->hide_tables();

	// Disable the time bar
	//get_canvas_view()->set_sensitive_timebar(false);

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateGradient_Context::on_user_click));

	App::toolbox->refresh();
}

void
StateGradient_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Gradient Tool"));
	App::dialog_tool_options->set_name("gradient");
}

Smach::event_result
StateGradient_Context::event_refresh_tool_options(const Smach::event& x)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateGradient_Context::~StateGradient_Context()
{
	save_settings();

	// Restore layer clicking
//	get_work_area()->allow_layer_clicks=prev_workarea_layer_status_;
	get_work_area()->allow_layer_clicks=true;
	get_canvas_view()->work_area->reset_cursor();

	App::dialog_tool_options->clear();

	// Enable the time bar
	//get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	//if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	//get_canvas_view()->show_tables();

	get_work_area()->refresh_cursor();

	App::toolbox->refresh();
}

Smach::event_result
StateGradient_Context::event_stop_handler(const Smach::event& x)
{
	throw Smach::egress_exception();
}

Smach::event_result
StateGradient_Context::event_refresh_handler(const Smach::event& x)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

void
StateGradient_Context::make_gradient(const Point& _p1, const Point& _p2)
{
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Gradient"));
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

	switch(get_type())
	{
	case GRADIENT_INTERPOLATION_LINEAR:

		layer=get_canvas_interface()->add_layer_to("linear_gradient",canvas,depth);
		layer->set_param("p1",p1);
		get_canvas_interface()->signal_layer_param_changed()(layer,"p1");
		layer->set_param("p2",p2);
		get_canvas_interface()->signal_layer_param_changed()(layer,"p2");
		break;
	case GRADIENT_RADIAL:
		layer=get_canvas_interface()->add_layer_to("radial_gradient",canvas,depth);
		layer->set_param("center",p1);
		get_canvas_interface()->signal_layer_param_changed()(layer,"center");
		layer->set_param("radius",(p2-p1).mag());
		get_canvas_interface()->signal_layer_param_changed()(layer,"radius");
		break;
	case GRADIENT_CONICAL:
		layer=get_canvas_interface()->add_layer_to("conical_gradient",canvas,depth);
		layer->set_param("center",p1);
		get_canvas_interface()->signal_layer_param_changed()(layer,"center");
		{
			Vector diff(p2-p1);
			layer->set_param("angle",Angle::tan(diff[1],diff[0]));
			get_canvas_interface()->signal_layer_param_changed()(layer,"angle");
		}
		break;
	case GRADIENT_SPIRAL:
		layer=get_canvas_interface()->add_layer_to("spiral_gradient",canvas,depth);
		layer->set_param("center",p1);
		get_canvas_interface()->signal_layer_param_changed()(layer,"center");
		layer->set_param("radius",(p2-p1).mag());
		get_canvas_interface()->signal_layer_param_changed()(layer,"radius");
		{
			Vector diff(p2-p1);
			layer->set_param("angle",Angle::tan(diff[1],diff[0]));
			get_canvas_interface()->signal_layer_param_changed()(layer,"angle");
		}
		break;

	default:
		return;
	}

	layer->set_param("blend_method",get_blend());
	get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

	layer->set_description(get_id());
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	no_egress_on_selection_change=true;
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	get_canvas_interface()->get_selection_manager()->set_selected_layer(layer);
	no_egress_on_selection_change=false;

	reset();
	increment_id();
}

Smach::event_result
StateGradient_Context::event_mouse_click_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_DOWN && event.button==BUTTON_LEFT)
	{
		point_holder=get_work_area()->snap_point_to_grid(event.pos);
		etl::handle<Duck> duck=new Duck();
		duck->set_point(point_holder);
		duck->set_name("p1");
		duck->set_type(Duck::TYPE_POSITION);
		get_work_area()->add_duck(duck);

		point2_duck=new Duck();
		point2_duck->set_point(point_holder);
		point2_duck->set_name("p2");
		point2_duck->set_type(Duck::TYPE_POSITION);
		get_work_area()->add_duck(point2_duck);

		handle<Duckmatic::Bezier> bezier(new Duckmatic::Bezier());
		bezier->p1=bezier->c1=duck;
		bezier->p2=bezier->c2=point2_duck;
		get_work_area()->add_bezier(bezier);

		return Smach::RESULT_ACCEPT;
	}

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_DRAG && event.button==BUTTON_LEFT)
	{
		point2_duck->set_point(get_work_area()->snap_point_to_grid(event.pos));
		get_work_area()->queue_draw();
		return Smach::RESULT_ACCEPT;
	}

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_UP && event.button==BUTTON_LEFT)
	{
		make_gradient(point_holder, get_work_area()->snap_point_to_grid(event.pos));
		get_work_area()->clear_ducks();
		return Smach::RESULT_ACCEPT;
	}

	return Smach::RESULT_OK;
}


void
StateGradient_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();
}
