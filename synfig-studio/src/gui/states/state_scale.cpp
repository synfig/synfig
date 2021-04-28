/* === S Y N F I G ========================================================= */
/*!	\file state_scale.cpp
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

#include <gui/states/state_scale.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/duck.h>
#include <gui/localization.h>
#include <gui/states/state_normal.h>
#include <gui/workarea.h>

#include <synfig/general.h>

#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

const int GAP = 3;

/* === G L O B A L S ======================================================= */

StateScale studio::state_scale;

/* === C L A S S E S & S T R U C T S ======================================= */

class DuckDrag_Scale : public DuckDrag_Base
{

	synfig::Vector last_scale;
	synfig::Vector drag_offset;
	synfig::Vector center;
	synfig::Vector snap;

	std::vector<synfig::Vector> positions;

	bool move_only;

	bool bad_drag;
public:
	bool lock_aspect;
	DuckDrag_Scale();
	void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector);
};


class studio::StateScale_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	synfigapp::Settings& settings;

	etl::handle<DuckDrag_Scale> duck_dragger_;

	Gtk::Grid options_grid;
	Gtk::Label title_label;

	Gtk::Label aspect_lock_label;
	Gtk::CheckButton aspect_lock_checkbutton;
	Gtk::Box aspect_lock_box;

public:

	bool get_aspect_lock_flag()const { return aspect_lock_checkbutton.get_active(); }
	void set_aspect_lock_flag(bool x) { aspect_lock_checkbutton.set_active(x); refresh_aspect_lock_flag(); }

	void refresh_aspect_lock_flag() { if(duck_dragger_)duck_dragger_->lock_aspect=get_aspect_lock_flag(); }

	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	void refresh_tool_options();

	StateScale_Context(CanvasView* canvas_view);

	~StateScale_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	void load_settings();
	void save_settings();
};	// END of class StateScale_Context

/* === M E T H O D S ======================================================= */

StateScale::StateScale():
	Smach::state<StateScale_Context>("scale")
{
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateScale_Context::event_refresh_tool_options));
	insert(event_def(EVENT_STOP,&StateScale_Context::event_stop_handler));
}

StateScale::~StateScale()
{
}

void* StateScale::enter_state(studio::CanvasView* machine_context) const
{
	return new StateScale_Context(machine_context);
}

void
StateScale_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		String value;

		if(settings.get_value("scale.lock_aspect",value) && value=="0")
			set_aspect_lock_flag(false);
		else
			set_aspect_lock_flag(true);
	}
	catch(...)
	{
		synfig::warning("State Scale: Caught exception when attempting to load settings.");
	}
}

void
StateScale_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		settings.set_value("scale.lock_aspect",get_aspect_lock_flag()?"1":"0");
	}
	catch(...)
	{
		synfig::warning("State Scale: Caught exception when attempting to save settings.");
	}
}

StateScale_Context::StateScale_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	duck_dragger_(new DuckDrag_Scale())
{
	// Toolbox widgets
	title_label.set_label(_("Scale Tool"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_hexpand();
	title_label.set_halign(Gtk::ALIGN_START);
	title_label.set_valign(Gtk::ALIGN_CENTER);

	aspect_lock_label.set_label(_("Lock Aspect Ratio"));
	aspect_lock_label.set_hexpand();
	aspect_lock_label.set_halign(Gtk::ALIGN_START);
	aspect_lock_label.set_valign(Gtk::ALIGN_CENTER);
	aspect_lock_box.pack_start(aspect_lock_label, true, true, 0);
	aspect_lock_box.pack_start(aspect_lock_checkbutton, false, false, 0);

	// Toolbox layout
	options_grid.attach(title_label,
		0, 0, 2, 1);
	options_grid.attach(aspect_lock_box,
		0, 1, 2, 1);

	aspect_lock_checkbutton.signal_toggled().connect(sigc::mem_fun(*this,&StateScale_Context::refresh_aspect_lock_flag));

	options_grid.set_vexpand(false);
	options_grid.set_border_width(GAP*2);
	options_grid.set_row_spacing(GAP);
	options_grid.show_all();

	refresh_tool_options();
	App::dialog_tool_options->present();

	get_work_area()->set_allow_layer_clicks(true);
	get_work_area()->set_duck_dragger(duck_dragger_);

	get_work_area()->set_cursor(Gdk::SIZING);
//	get_work_area()->reset_cursor();

	App::dock_toolbox->refresh();

	set_aspect_lock_flag(true);
	load_settings();
}

void
StateScale_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_grid);
	App::dialog_tool_options->set_local_name(_("Scale Tool"));
	App::dialog_tool_options->set_name("scale");
}

Smach::event_result
StateScale_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateScale_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw &state_normal;
	return Smach::RESULT_OK;
}

StateScale_Context::~StateScale_Context()
{
	save_settings();

	get_work_area()->clear_duck_dragger();
	get_work_area()->reset_cursor();

	App::dialog_tool_options->clear();

	App::dock_toolbox->refresh();
}




DuckDrag_Scale::DuckDrag_Scale():
	move_only(),
	bad_drag(),
	lock_aspect(true)
{ }

#ifndef EPSILON
#define EPSILON	0.0000001
#endif

void
DuckDrag_Scale::begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& offset)
{
	last_scale=Vector(1,1);
	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	//if(duckmatic->get_selected_ducks().size()<2)
	//{
	//	bad_drag=true;
//		return;
//	}
	bad_drag=false;

		drag_offset=duckmatic->find_duck(offset)->get_trans_point();

		//snap=drag_offset-duckmatic->snap_point_to_grid(drag_offset);
		//snap=offset-drag_offset;
		snap=Vector(0,0);

	// Calculate center
	Point vmin(100000000,100000000);
	Point vmax(-100000000,-100000000);
	//std::set<etl::handle<Duck> >::iterator iter;
	positions.clear();
	int i;
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		Point p((*iter)->get_trans_point());
		vmin[0]=min(vmin[0],p[0]);
		vmin[1]=min(vmin[1],p[1]);
		vmax[0]=max(vmax[0],p[0]);
		vmax[1]=max(vmax[1],p[1]);
		positions.push_back(p);
	}
	if((vmin-vmax).mag()<=EPSILON)
		move_only=true;
	else
		move_only=false;

	center=(vmin+vmax)*0.5;
}


void
DuckDrag_Scale::duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector)
{
	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	if(bad_drag)
		return;

	//std::set<etl::handle<Duck> >::iterator iter;
	synfig::Vector vect(duckmatic->snap_point_to_grid(vector)-center);
	last_scale=vect;

	if(move_only)
	{
		int i;
		for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		{
			if(((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION))continue;

			Vector p(positions[i]);

			p[0]+=vect[0];
			p[1]+=vect[1];
			(*iter)->set_trans_point(p);
		}
		for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		{
			if(!((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION))continue;

			Vector p(positions[i]);

			p[0]+=vect[0];
			p[1]+=vect[1];
			(*iter)->set_trans_point(p);
		}
		return;
	}

	if(!lock_aspect)
	{
		if(abs(drag_offset[0]-center[0])>EPSILON)
			vect[0]/=drag_offset[0]-center[0];
		else
			vect[0]=1;
		if(abs(drag_offset[1]-center[1])>EPSILON)
			vect[1]/=drag_offset[1]-center[1];
		else
			vect[1]=1;
		}
	else
	{
		//vect[0]=vect[1]=vect.mag()*0.707106781;
		Real amount(vect.mag()/(drag_offset-center).mag());
		vect[0]=vect[1]=amount;
	}

	if(vect[0]<EPSILON && vect[0]>-EPSILON)
		vect[0]=1;
	if(vect[1]<EPSILON && vect[1]>-EPSILON)
		vect[1]=1;

	int i;
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		if(((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION))continue;

		Vector p(positions[i]-center);

		p[0]*=vect[0];
		p[1]*=vect[1];
		p+=center;
		(*iter)->set_trans_point(p);
	}
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		if(!((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION))continue;

		Vector p(positions[i]-center);

		p[0]*=vect[0];
		p[1]*=vect[1];
		p+=center;
		(*iter)->set_trans_point(p);
	}

	last_scale=vect;
	//snap=Vector(0,0);
}

bool
DuckDrag_Scale::end_duck_drag(Duckmatic* duckmatic)
{
	if(bad_drag)return false;

	if((last_scale-Vector(1,1)).mag()>0.0001)
	{
		duckmatic->signal_edited_selected_ducks();
		return true;
	}
	else
	{
		duckmatic->signal_user_click_selected_ducks(0);
		return false;
	}
}
