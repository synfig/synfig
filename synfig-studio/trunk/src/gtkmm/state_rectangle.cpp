/* === S I N F G =========================================================== */
/*!	\file state_gradient.cpp
**	\brief Template File
**
**	$Id: state_rectangle.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include <sinfg/valuenode_dynamiclist.h>
#include <sinfgapp/action_system.h>

#include "state_rectangle.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"

#include <sinfgapp/action.h>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "toolbox.h"
#include "dialog_tooloptions.h"
#include <gtkmm/optionmenu.h>
#include "duck.h"
#include <sinfgapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StateRectangle studio::state_rectangle;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateRectangle_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;
	
	Duckmatic::Push duckmatic_push;
	
	Point point_holder;
	
	etl::handle<Duck> point2_duck;

	void refresh_ducks();
	
	bool prev_workarea_layer_status_;
		
	//Toolbox settings
	sinfgapp::Settings& settings;
	
	//Toolbox display
	Gtk::Table options_table;
	
	Gtk::Entry		entry_id; //what to name the layer
		
	Gtk::Adjustment	adj_expand;
	Gtk::SpinButton	spin_expand;
	
	Gtk::CheckButton check_invert;
	
public:

	sinfg::String get_id()const { return entry_id.get_text(); }
	void set_id(const sinfg::String& x) { return entry_id.set_text(x); }

	Real get_expand()const { return adj_expand.get_value(); }
	void set_expand(Real f) { adj_expand.set_value(f); }
	
	bool get_invert()const { return check_invert.get_active(); }
	void set_invert(bool i) { check_invert.set_active(i); }
	
	void refresh_tool_options(); //to refresh the toolbox	

	//events
	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	//constructor destructor
	StateRectangle_Context(CanvasView* canvas_view);
	~StateRectangle_Context();

	//Canvas interaction
	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<sinfgapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	sinfg::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
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

	void make_rectangle(const Point& p1, const Point& p2);
	
};	// END of class StateGradient_Context

/* === M E T H O D S ======================================================= */

StateRectangle::StateRectangle():
	Smach::state<StateRectangle_Context>("rectangle")
{
	insert(event_def(EVENT_STOP,&StateRectangle_Context::event_stop_handler));
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StateRectangle_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_REFRESH,&StateRectangle_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateRectangle_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateRectangle_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateRectangle_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateRectangle_Context::event_mouse_click_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateRectangle_Context::event_refresh_tool_options));
}

StateRectangle::~StateRectangle()
{
}

void
StateRectangle_Context::load_settings()
{	
	String value;
	
	//parse the arguments yargh!
	if(settings.get_value("rectangle.id",value))
		set_id(value);
	else
		set_id("Rectangle");

	if(settings.get_value("rectangle.expand",value))
		set_expand(atof(value.c_str()));
	else
		set_expand(0);
	
	if(settings.get_value("rectangle.invert",value) && value != "0")
		set_invert(true);
	else
		set_invert(false);
}

void
StateRectangle_Context::save_settings()
{	
	settings.set_value("rectangle.id",get_id().c_str());
	settings.set_value("rectangle.expand",strprintf("%f",get_expand()));
	settings.set_value("rectangle.invert",get_invert()?"1":"0");
}

void
StateRectangle_Context::reset()
{
	refresh_ducks();
}

void
StateRectangle_Context::increment_id()
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

StateRectangle_Context::StateRectangle_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	duckmatic_push(get_work_area()),
	prev_workarea_layer_status_(get_work_area()->allow_layer_clicks),
	settings(sinfgapp::Main::get_selected_input_device()->settings()),
	entry_id(),
	adj_expand(0,0,1,0.01,0.1),
	spin_expand(adj_expand,0.1,3),
	check_invert(_("Invert"))
{
	no_egress_on_selection_change=false;
	load_settings();
	
	// Set up the tool options dialog
	//options_table.attach(*manage(new Gtk::Label(_("Circle Tool"))), 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	options_table.attach(entry_id, 0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	//expand stuff
	options_table.attach(*manage(new Gtk::Label(_("Expansion:"))), 0, 1, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_expand, 1, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
		
	//invert flag
	options_table.attach(check_invert, 1, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	
	options_table.show_all();
	
	//App::dialog_tool_options->set_widget(options_table);
	refresh_tool_options();
	App::dialog_tool_options->present();

	// Turn off layer clicking
	get_work_area()->allow_layer_clicks=false;
	
	// clear out the ducks
	get_work_area()->clear_ducks();
	
	// Refresh the work area
	get_work_area()->queue_draw();

	get_canvas_view()->work_area->set_cursor(Gdk::CROSSHAIR);

	// Hide the tables if they are showing
	//prev_table_status=get_canvas_view()->tables_are_visible();
	//if(prev_table_status)get_canvas_view()->hide_tables();
		
	// Hide the time bar
	//get_canvas_view()->hide_timebar();
	
	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateRectangle_Context::on_user_click));

	App::toolbox->refresh();
}

void
StateRectangle_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Rectangle Tool"));
	App::dialog_tool_options->set_name("rectangle");
}

Smach::event_result
StateRectangle_Context::event_refresh_tool_options(const Smach::event& x)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateRectangle_Context::~StateRectangle_Context()
{
	save_settings();

	// Restore layer clicking
	get_work_area()->allow_layer_clicks = prev_workarea_layer_status_;

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
StateRectangle_Context::event_stop_handler(const Smach::event& x)
{
	throw Smach::egress_exception();
}

Smach::event_result
StateRectangle_Context::event_refresh_handler(const Smach::event& x)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

void
StateRectangle_Context::make_rectangle(const Point& _p1, const Point& _p2)
{
	sinfgapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Rectangle"));
	sinfgapp::PushMode push_mode(get_canvas_interface(),sinfgapp::MODE_NORMAL);

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

	//create the layer
	layer=get_canvas_interface()->add_layer_to("rectangle",canvas,depth);

	const sinfg::TransformStack& transform(get_canvas_view()->get_curr_transform_stack());
	const Point p1(transform.unperform(_p1));
	const Point p2(transform.unperform(_p2));
	
	//set all the parameters
	layer->set_param("point1",p1);
	get_canvas_interface()->signal_layer_param_changed()(layer,"point1");
	layer->set_param("point2",p2);
	get_canvas_interface()->signal_layer_param_changed()(layer,"point2");
	
	layer->set_param("expand",get_expand());
	get_canvas_interface()->signal_layer_param_changed()(layer,"expand");

	layer->set_param("invert",get_invert());
	get_canvas_interface()->signal_layer_param_changed()(layer,"invert");
	
	//name
	layer->set_description(get_id());
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	no_egress_on_selection_change=true;
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	get_canvas_interface()->get_selection_manager()->set_selected_layer(layer);
	no_egress_on_selection_change=false;

	//post clean up stuff...
	reset();
	increment_id();
}

Smach::event_result
StateRectangle_Context::event_mouse_click_handler(const Smach::event& x)
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
		point2_duck->set_box_duck(duck);
		get_work_area()->add_duck(point2_duck);

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
		make_rectangle(point_holder, get_work_area()->snap_point_to_grid(event.pos));
		get_work_area()->clear_ducks();
		return Smach::RESULT_ACCEPT;
	}

	return Smach::RESULT_OK;
}


void
StateRectangle_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();
}
