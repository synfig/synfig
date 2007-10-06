/* === S Y N F I G ========================================================= */
/*!	\file state_width.cpp
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

#include <ETL/bezier>

#include <synfig/valuenode_dynamiclist.h>
#include <synfigapp/action_system.h>

#include "state_width.h"
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

//#include <synfigapp/value_desc.h>
#include <synfigapp/main.h>

#include <ETL/clock>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StateWidth studio::state_width;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateWidth_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	//Point mouse_pos;

	handle<Duck> center;
	handle<Duck> radius;
	handle<Duck> closestpoint;

	map<handle<Duck>,Real>	changetable;

	etl::clock	clocktime;
	Real		lastt;

	bool added;

	void refresh_ducks();

	bool prev_workarea_layer_clicking;
	bool prev_workarea_duck_clicking;
	Duckmatic::Type old_duckmask;

	//Toolbox settings
	synfigapp::Settings& settings;

	//Toolbox display
	Gtk::Table options_table;

	//Gtk::Entry		entry_id; //what to name the layer

	Gtk::Adjustment	adj_delta;
	Gtk::SpinButton	spin_delta;

	Gtk::Adjustment	adj_radius;
	Gtk::SpinButton	spin_radius;

	Gtk::CheckButton check_relative;

	void AdjustWidth(handle<Duckmatic::Bezier> c, float t, Real mult, bool invert);

public:

	Real get_delta()const { return adj_delta.get_value(); }
	void set_delta(Real f) { adj_delta.set_value(f); }

	Real get_radius()const { return adj_radius.get_value(); }
	void set_radius(Real f) { adj_radius.set_value(f); }

	bool get_relative() const { return check_relative.get_active(); }
	void set_relative(bool r) { check_relative.set_active(r); }

	void refresh_tool_options(); //to refresh the toolbox

	//events
	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_mouse_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	//constructor destructor
	StateWidth_Context(CanvasView* canvas_view);
	~StateWidth_Context();

	//Canvas interaction
	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	//Modifying settings etc.
	void load_settings();
	void save_settings();
	void reset();

};	// END of class StateGradient_Context

/* === M E T H O D S ======================================================= */

StateWidth::StateWidth():
	Smach::state<StateWidth_Context>("width")
{
	insert(event_def(EVENT_STOP,&StateWidth_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateWidth_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateWidth_Context::event_mouse_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateWidth_Context::event_mouse_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateWidth_Context::event_mouse_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateWidth_Context::event_refresh_tool_options));
}

StateWidth::~StateWidth()
{
}

void
StateWidth_Context::load_settings()
{
	String value;

	//parse the arguments yargh!
	if(settings.get_value("width.delta",value))
		set_delta(atof(value.c_str()));
	else
		set_delta(6);

	if(settings.get_value("width.radius",value))
		set_radius(atof(value.c_str()));
	else
		set_radius(15);

	//defaults to true
	if(settings.get_value("width.relative",value) && value == "0")
		set_relative(false);
	else
		set_relative(true);
}

void
StateWidth_Context::save_settings()
{
	settings.set_value("width.delta",strprintf("%f",get_delta()));
	settings.set_value("width.radius",strprintf("%f",get_radius()));
	settings.set_value("width.relative",get_relative()?"1":"0");
}

void
StateWidth_Context::reset()
{
	refresh_ducks();
}

StateWidth_Context::StateWidth_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	prev_workarea_layer_clicking(get_work_area()->get_allow_layer_clicks()),
	prev_workarea_duck_clicking(get_work_area()->get_allow_duck_clicks()),
	old_duckmask(get_work_area()->get_type_mask()),

	settings(synfigapp::Main::get_selected_input_device()->settings()),

	adj_delta(6,0,1,0.001,0.01),
	spin_delta(adj_delta,0.01,3),

	adj_radius(0,0,1e50,1,10),
	spin_radius(adj_radius,1,1),

	check_relative(_("Relative Growth"))
{
	load_settings();

	// Set up the tool options dialog
	//options_table.attach(*manage(new Gtk::Label(_("Width Tool"))), 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	//options_table.attach(entry_id, 0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	//expand stuff
	options_table.attach(*manage(new Gtk::Label(_("Growth:"))), 0, 1, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_delta, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(*manage(new Gtk::Label(_("Radius:"))), 0, 1, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(spin_radius, 1, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.attach(check_relative, 0, 2, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.show_all();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// clear out the ducks
	//get_work_area()->clear_ducks();

	// Refresh the work area
	get_work_area()->queue_draw();

	//Create the new ducks
	added = false;

	if(!center)
	{
		center = new Duck();
		center->set_name("p1");
		center->set_type(Duck::TYPE_POSITION);
	}

	if(!radius)
	{
		radius = new Duck();
		radius->set_origin(center);
		radius->set_radius(true);
		radius->set_type(Duck::TYPE_RADIUS);
		radius->set_name("radius");
	}

	if(!closestpoint)
	{
		closestpoint = new Duck();
		closestpoint->set_name("closest");
		closestpoint->set_type(Duck::TYPE_POSITION);
	}

	//Disable duck clicking for the maximum coolness :)
	get_work_area()->set_allow_duck_clicks(false);
	get_work_area()->set_type_mask((Duck::Type)((int)Duck::TYPE_WIDTH + (int)Duck::TYPE_RADIUS));

	// Turn the mouse pointer to crosshairs
	get_work_area()->set_cursor(Gdk::CROSSHAIR);

	// Hide the tables if they are showing
	//prev_table_status=get_canvas_view()->tables_are_visible();
	//if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	//get_canvas_view()->set_sensitive_timebar(false);

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateWidth_Context::on_user_click));

	App::toolbox->refresh();
}

void
StateWidth_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Width Tool"));
	App::dialog_tool_options->set_name("width");
}

Smach::event_result
StateWidth_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateWidth_Context::~StateWidth_Context()
{
	save_settings();

	//remove ducks if need be
	if(added)
	{
		get_work_area()->erase_duck(center);
		get_work_area()->erase_duck(radius);
		get_work_area()->erase_duck(closestpoint);
		added = false;
	}

	// Restore Duck clicking
	get_work_area()->set_allow_duck_clicks(prev_workarea_duck_clicking);

	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_clicking);

	// Restore the mouse pointer
	get_work_area()->reset_cursor();

	// Restore duck masking
	get_work_area()->set_type_mask(old_duckmask);

	// Tool options be rid of ye!!
	App::dialog_tool_options->clear();

	// Enable the time bar
	//get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	//if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	App::toolbox->refresh();
}

Smach::event_result
StateWidth_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw Smach::egress_exception();
}

Smach::event_result
StateWidth_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

void
StateWidth_Context::AdjustWidth(handle<Duckmatic::Bezier> c, float t, Real mult, bool invert)
{
	//Leave the function if there is no curve
	if(!c)return;

	Real amount1=0,amount2=0;

	//decide how much to change each width
	/*
	t \in [0,1]

	both pressure and multiply amount are in mult
		(may want to change this to allow different types of falloff)

	rsq is the squared distance from the point on the curve (also part of the falloff)


	*/
	//may want to provide a different falloff function...
	if(t <= 0.2)
		amount1 = mult;
	else if(t >= 0.8)
		amount2 = mult;
	else
	{
		t = (t-0.2)/0.6;
		amount1 = (1-t)*mult;
		amount2 = t*mult;
	}

	if(invert)
	{
		amount1 *= -1;
		amount2 *= -1;
	}

	handle<Duck>	p1 = c->p1;
	handle<Duck>	p2 = c->p2;

	handle<Duck>	w1,w2;

	//find w1,w2
	{
		const DuckList dl = get_work_area()->get_duck_list();

		DuckList::const_iterator i = dl.begin();

		for(;i != dl.end(); ++i)
		{
			if((*i)->get_type() == Duck::TYPE_WIDTH)
			{
				if((*i)->get_origin_duck() == p1)
				{
					w1 = *i;
				}

				if((*i)->get_origin_duck() == p2)
				{
					w2 = *i;
				}
			}
		}
	}

	if(amount1 != 0 && w1)
	{
		Real width = w1->get_point().mag();

		width += amount1;
		w1->set_point(Vector(width,0));

		//log in the list of changes...
		//to truly be changed after everything is said and done
		changetable[w1] = width;
	}

	if(amount2 != 0 && w2)
	{
		Real width = w2->get_point().mag();

		width += amount2;
		w2->set_point(Vector(width,0));

		//log in the list of changes...
		//to truly be changed after everything is said and done
		changetable[w2] = width;
	}
}

Smach::event_result
StateWidth_Context::event_mouse_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	//handle ze click
	if( (event.key == EVENT_WORKAREA_MOUSE_BUTTON_DOWN || event.key == EVENT_WORKAREA_MOUSE_BUTTON_DRAG)
			&& event.button == BUTTON_LEFT )
	{
		const Real pw = get_work_area()->get_pw();
		const Real ph = get_work_area()->get_ph();
		const Real scale = sqrt(pw*pw+ph*ph);
		const Real rad = get_relative() ? scale * get_radius() : get_radius();

		bool invert = (event.modifier&Gdk::CONTROL_MASK);

		const Real threshold = 0.08;

		float t = 0;
		Real rsq = 0;

		Real dtime = 1/60.0;

		//if we're dragging get the difference in time between now and then
		if(event.key == EVENT_WORKAREA_MOUSE_BUTTON_DRAG)
		{
			dtime = min(1/15.0,clocktime());
		}
		clocktime.reset();

		//make way for new ducks
		//get_work_area()->clear_ducks();

		//update positions
		//mouse_pos = event.pos;

		center->set_point(event.pos);
		if(!added)get_work_area()->add_duck(center);

		radius->set_scalar(rad);
		if(!added)get_work_area()->add_duck(radius);

		//the other duck is at the current duck
		closestpoint->set_point(event.pos);
		if(!added)get_work_area()->add_duck(closestpoint);

		//get the closest curve...
		handle<Duckmatic::Bezier>	c;
		if(event.pressure >= threshold)
			c = get_work_area()->find_bezier(event.pos,scale*8,rad,&t);

		//run algorithm on event.pos to get 2nd placement
		if(!c.empty())
		{
			bezier<Point> curve;
			Point p;

			curve[0] = c->p1->get_trans_point();
			curve[1] = c->c1->get_trans_point();
			curve[2] = c->c2->get_trans_point();
			curve[3] = c->p2->get_trans_point();

			p = curve(t);
			rsq = (p-event.pos).mag_squared();

			const Real r = rad*rad;

			if(rsq < r)
			{
				closestpoint->set_point(curve(t));

				//adjust the width...
				//squared falloff for radius... [0,1]

				Real ri = (r - rsq)/r;
				AdjustWidth(c,t,ri*event.pressure*get_delta()*dtime,invert);
			}
		}

		//the points have been added
		added = true;

		//draw where it is yo!
		get_work_area()->queue_draw();

		return Smach::RESULT_ACCEPT;
	}

	if(event.key == EVENT_WORKAREA_MOUSE_BUTTON_UP && event.button == BUTTON_LEFT)
	{
		if(added)
		{
			get_work_area()->erase_duck(center);
			get_work_area()->erase_duck(radius);
			get_work_area()->erase_duck(closestpoint);
			added = false;
		}

		//Affect the width changes here...
		map<handle<Duck>,Real>::iterator i = changetable.begin();

		synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Sketch Width"));
		for(; i != changetable.end(); ++i)
		{
			//for each duck modify IT!!!
			ValueDesc desc = i->first->get_value_desc();

			if(	desc.get_value_type() == ValueBase::TYPE_REAL )
			{
				Action::Handle action(Action::create("value_desc_set"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());

				action->set_param("value_desc",desc);
				action->set_param("new_value",ValueBase(i->second));
				action->set_param("time",get_canvas_view()->get_time());

				if(!action->is_ready() || !get_canvas_view()->get_instance()->perform_action(action))
				{
					group.cancel();
					synfig::warning("Changing the width action has failed");
					return Smach::RESULT_ERROR;
				}
			}
		}

		changetable.clear();

		get_work_area()->queue_draw();

		return Smach::RESULT_ACCEPT;
	}

	return Smach::RESULT_OK;
}


void
StateWidth_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();
}
