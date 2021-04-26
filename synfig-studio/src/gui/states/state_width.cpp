/* === S Y N F I G ========================================================= */
/*!	\file state_width.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#include <ETL/clock>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/duck.h>
#include <gui/event_mouse.h>
#include <gui/localization.h>
#include <gui/states/state_normal.h>
#include <gui/states/state_width.h>
#include <gui/widgets/widget_distance.h>
#include <gui/workarea.h>

#include <synfig/blinepoint.h>
#include <synfig/general.h>
#include <synfig/valuenodes/valuenode_wplist.h>

#include <synfigapp/action.h>
#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace studio;

/* === M A C R O S ========================================================= */

const int GAP = 3;

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
	// Real		lastt; // unused

	bool added;

	void refresh_ducks();

	WorkArea::PushState push_state;

	// Toolbox settings
	synfigapp::Settings& settings;

	Gtk::Grid options_grid;
	Gtk::Label title_label;

	Glib::RefPtr<Gtk::Adjustment> adj_delta;
	Gtk::SpinButton	spin_delta;

	Widget_Distance *influence_radius;

	Gtk::Label relative_label;
	Gtk::CheckButton relative_checkbutton;
	Gtk::Grid relative_grid;

	Gtk::Label growth_label;
	Gtk::Label radius_label;

	void AdjustWidth(handle<Duckmatic::Bezier> c, float t, Real mult, bool invert);

public:

	Real get_delta()const { return adj_delta->get_value(); }
	void set_delta(Real f) { adj_delta->set_value(f); }

	Real get_radius()const { return influence_radius->get_value().get(Distance::SYSTEM_UNITS,get_canvas_view()->get_canvas()->rend_desc());}
	void set_radius(Distance f) { influence_radius->set_value(f); }

	bool get_relative() const { return relative_checkbutton.get_active(); }
	void set_relative(bool r) { relative_checkbutton.set_active(r); }

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

};	// END of class StateWidth_Context

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

void* StateWidth::enter_state(studio::CanvasView* machine_context) const
{
	return new StateWidth_Context(machine_context);
}

void
StateWidth_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		String value;

		//parse the arguments yargh!
		if(settings.get_value("width.delta",value))
			set_delta(atof(value.c_str()));
		else
			set_delta(6);

		if(settings.get_value("width.radius",value))
			set_radius(Distance(atof(value.c_str()), App::distance_system));
		else
			set_radius(Distance(60, App::distance_system));

		//defaults to false
		if(settings.get_value("width.relative",value) && value == "1")
			set_relative(true);
		else
			set_relative(false);
	}
	catch(...)
	{
		synfig::warning("State Width: Caught exception when attempting to load settings.");
	}
}

void
StateWidth_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		settings.set_value("width.delta",strprintf("%f",get_delta()));
		settings.set_value("width.radius",influence_radius->get_value().get_string());
		settings.set_value("width.relative",get_relative()?"1":"0");
	}
	catch(...)
	{
		synfig::warning("State Width: Caught exception when attempting to save settings.");
	}
}

void
StateWidth_Context::reset()
{
	refresh_ducks();
}

StateWidth_Context::StateWidth_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	push_state(*get_work_area()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	adj_delta(Gtk::Adjustment::create(6,0,20,0.01,0.1)),
	spin_delta(adj_delta,0.01,3)
{
	// Toolbox widgets
	title_label.set_label(_("Width Tool"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_hexpand();
	title_label.set_halign(Gtk::ALIGN_START);
	title_label.set_valign(Gtk::ALIGN_CENTER);
	
	relative_label.set_label(_("Relative Growth"));
	relative_label.set_halign(Gtk::ALIGN_START);
	relative_label.set_valign(Gtk::ALIGN_CENTER);
	relative_label.set_hexpand();
	
	relative_grid.attach(relative_label, 0, 0);
	relative_grid.attach_next_to(relative_checkbutton, Gtk::POS_RIGHT, 1, 1);

	growth_label.set_label(_("Growth:"));
	growth_label.set_halign(Gtk::ALIGN_START);
	growth_label.set_valign(Gtk::ALIGN_CENTER);

	radius_label.set_label(_("Radius:"));
	radius_label.set_halign(Gtk::ALIGN_START);
	radius_label.set_valign(Gtk::ALIGN_CENTER);

	influence_radius=manage(new Widget_Distance());
	influence_radius->show();
	influence_radius->set_digits(0);
	influence_radius->set_range(0,10000000);
	influence_radius->set_size_request(24,-1);

	load_settings();

	// Toolbox layout
	options_grid.attach(title_label,
		0, 0, 2, 1);
	options_grid.attach(growth_label,
		0, 1, 1, 1);
	options_grid.attach(spin_delta,
		1, 1, 1, 1);
	options_grid.attach(radius_label,
		0, 2, 1, 1);
	options_grid.attach(*influence_radius,
		1, 2, 1, 1);
	options_grid.attach(relative_grid,
		0, 3, 2, 1);

	options_grid.set_vexpand(false);
	options_grid.set_border_width(GAP*2);
	options_grid.set_row_spacing(GAP);
	options_grid.show_all();
	options_grid.set_margin_bottom(0);

	refresh_tool_options();
	App::dialog_tool_options->present();
	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);
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
		radius->set_point(Point(1.0,0.0));
	}
	if(!closestpoint)
	{
		closestpoint = new Duck();
		closestpoint->set_name("closest");
		closestpoint->set_type(Duck::TYPE_POSITION);
	}
	//Disable duck clicking for the maximum coolness :)
	get_work_area()->set_allow_duck_clicks(false);
	// Hide all tangent, vertex and angle ducks and show the width and
	// radius ducks
	get_work_area()->set_type_mask((get_work_area()->get_type_mask()-Duck::TYPE_TANGENT-Duck::TYPE_VERTEX-Duck::TYPE_ANGLE) | Duck::TYPE_WIDTH | Duck::TYPE_RADIUS);
	get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);

	// Turn the mouse pointer to crosshairs
	get_work_area()->set_cursor(Gdk::CROSSHAIR);

	App::dock_toolbox->refresh();
}

void
StateWidth_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_grid);
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

	// Tool options be rid of ye!!
	App::dialog_tool_options->clear();
	// Refresh the work area
	get_work_area()->queue_draw();

	App::dock_toolbox->refresh();
}

Smach::event_result
StateWidth_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw &state_normal;
	return Smach::RESULT_OK;
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
		float u = (t-0.2)/0.6;
		amount1 = (1-u)*mult;
		amount2 = u*mult;
	}
	// change sign if we are decreasing widths
	if(invert)
	{
		amount1 *= -1;
		amount2 *= -1;
	}
	// ducks for the bezier vertexes
	handle<Duck>	p1 = c->p1;
	handle<Duck>	p2 = c->p2;
	// ducks for the widths of the bezier vertexes
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
	// change the widths of the affected BLine of Outlines
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
	///////
	// Change the widths of the affected BLine of Advance Outlines
	// Parents value nodes of the value node of the p1 and p2 ducks.
	synfig::ValueNode::Handle p1pvn(p1->get_value_desc().get_parent_value_node());
	synfig::ValueNode::Handle p2pvn(p2->get_value_desc().get_parent_value_node());
	// if the bezier position ducks are linkable valuenode children
	if(p1pvn && p2pvn && p1pvn==p2pvn)
	{
		// we guess that the parent value node is a bline value node
		synfig::ValueNode::Handle bezier_bline=p1pvn;
		// Positions of the points on the bline
		Real p1_pos, bezier_size;
		// index of the first point on the bezier
		int p1_i;
		// retrieve the number of blinepoints on the bline and the loop of the bline
		int bline_size((*(bezier_bline))(get_canvas()->get_time()).get_list().size());
		bool loop((*(bezier_bline))(get_canvas()->get_time()).get_loop());
		p1_i = p1->get_value_desc().get_index();
		// bezier size depends on loop status
		bezier_size = 1.0/(loop?bline_size:(bline_size-1));
		if(loop)
		{
			// if looped and the we are in the first bezier
			if(p1_i == (bline_size -1))
				p1_i = 0;
			else
				p1_i++;
		}
		// the position is based on the index and the bezier size
		p1_pos = Real(p1_i)*bezier_size;
		// find all the widthpoints
		const DuckList dl = get_work_area()->get_duck_list();
		DuckList::const_iterator i = dl.begin();
		for(;i != dl.end(); ++i)
		{
			handle<Duck> iduck(*i);
			handle<Duck> iduck_origin(iduck->get_origin_duck());
			// If we find a width duck
			if(iduck->get_type() == Duck::TYPE_WIDTH && iduck_origin)
			{
				// if it has an origin duck
				synfigapp::ValueDesc origin_value_desc(iduck_origin->get_value_desc());
				ValueNode::Handle wpvn(ValueNode::Handle::cast_dynamic(origin_value_desc.get_value_node()));
				// if the origin duck is widthpoint type and it belongs to a list
				if(wpvn && wpvn->get_type() == type_width_point && origin_value_desc.parent_is_linkable_value_node())
				{
					// and if the width point list that it belongs to...
					ValueNode_WPList::Handle wplist(ValueNode_WPList::Handle::cast_dynamic(origin_value_desc.get_parent_value_node()));
					if(wplist)
					{
						// ... has a bline valid and is the same as the bline
						// we found for the bezier previously caught...
						ValueNode::Handle bline(wplist->get_bline());
						if(bline && (bline==bezier_bline))
						{
							// ... then update the values properly
							synfig::WidthPoint wpoint((*wpvn)(get_canvas()->get_time()).get(synfig::WidthPoint()));
							Real pos(wpoint.get_norm_position(wplist->get_loop()));
							Real tpos(p1_pos+t*bezier_size);
							// The factor of 20 can be modified by the user as a preference.
							// The higher value the more local effect has the
							// Width Tool around the widths points.
							Real amount(mult*exp(-20.0*(fabs(pos-tpos)+0.000001)));
							amount*=invert?-1.0:1.0;
							Real width = iduck->get_point().mag();
							width += amount;
							iduck->set_point(Vector(width,0));
							changetable[iduck] = width;
						}
					}
				}
			}
		}
	}
}

Smach::event_result
StateWidth_Context::event_mouse_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	//handle the click
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
			dtime = min((float)(1/15.0),(float)(clocktime()));
		}
		clocktime.reset();

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
			curve.sync();
			p = curve(t);
			rsq = (p-event.pos).mag_squared();
			const Real r = rad*rad;
			if(rsq < r)
			{
				closestpoint->set_point(p);
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
			if (desc.get_value_type() == type_real) {
				Action::Handle action(Action::create("ValueDescSet"));
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
			else
			if (desc.get_value_type() == type_bline_point
			 && desc.parent_is_value_desc()
			 && desc.get_sub_name() == "width")
			{
				BLinePoint p;
				p.set_width(i->second);

				Action::Handle action(Action::create("ValueDescSet"));
				assert(action);

				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());

				action->set_param("value_desc",desc);
				action->set_param("new_value",ValueBase(p));
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
