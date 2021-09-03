/* === S Y N F I G ========================================================= */
/*!	\file state_rotate.cpp
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

#include <gui/states/state_rotate.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/duck.h>
#include <gui/localization.h>
#include <gui/workarea.h>

#include <synfig/angle.h>
#include <synfig/general.h>

#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef EPSILON
#define EPSILON	0.0000001
#endif

const int GAP = 3;

/* === G L O B A L S ======================================================= */

StateRotate studio::state_rotate;

/* === C L A S S E S & S T R U C T S ======================================= */

class DuckDrag_Rotate : public DuckDrag_Base
{

	synfig::Vector last_rotate;
	synfig::Vector drag_offset;
	synfig::Vector center;
	synfig::Vector snap;

	Angle original_angle;
	Real original_mag;

	std::vector<synfig::Vector> positions;


	bool bad_drag;
	bool move_only;

public:
	etl::handle<CanvasView> canvas_view_;
	bool use_magnitude;
	DuckDrag_Rotate();
	void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector);

	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
};


class studio::StateRotate_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	synfigapp::Settings& settings;

	etl::handle<DuckDrag_Rotate> duck_dragger_;

	Gtk::Grid options_grid;
	Gtk::Label title_label;

	Gtk::Label scale_label;
	Gtk::CheckButton scale_checkbutton;
	Gtk::Box scale_box;

public:

	bool get_scale_flag()const { return scale_checkbutton.get_active(); }
	void set_scale_flag(bool x) { scale_checkbutton.set_active(x); refresh_scale_flag(); }

	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	void refresh_tool_options();

	void refresh_scale_flag() { if(duck_dragger_)duck_dragger_->use_magnitude=get_scale_flag(); }

	StateRotate_Context(CanvasView* canvas_view);

	~StateRotate_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	void load_settings();
	void save_settings();
};	// END of class StateRotate_Context

/* === M E T H O D S ======================================================= */

StateRotate::StateRotate():
	Smach::state<StateRotate_Context>("rotate")
{
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateRotate_Context::event_refresh_tool_options));
	insert(event_def(EVENT_STOP,&StateRotate_Context::event_stop_handler));
}

StateRotate::~StateRotate()
{
}

void* StateRotate::enter_state(studio::CanvasView* machine_context) const
{
	return new StateRotate_Context(machine_context);
}

void
StateRotate_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		String value;

		if(settings.get_value("rotate.scale",value) && value=="0")
			set_scale_flag(false);
		else
			set_scale_flag(true);
	}
	catch(...)
	{
		synfig::warning("State Rotate: Caught exception when attempting to load settings.");
	}
}

void
StateRotate_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		settings.set_value("rotate.scale",get_scale_flag()?"1":"0");
	}
	catch(...)
	{
		synfig::warning("State Rotate: Caught exception when attempting to save settings.");
	}
}

StateRotate_Context::StateRotate_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	duck_dragger_(new DuckDrag_Rotate())
{
	duck_dragger_->canvas_view_=get_canvas_view();

	// Toolbox widgets
	title_label.set_label(_("Rotate Tool"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_hexpand();
	title_label.set_halign(Gtk::ALIGN_START);
	title_label.set_valign(Gtk::ALIGN_CENTER);

	scale_label.set_label(_("Allow Scale"));
	scale_label.set_halign(Gtk::ALIGN_START);
	scale_label.set_valign(Gtk::ALIGN_CENTER);

	scale_box.pack_start(scale_label, true, true, 0);
	scale_box.pack_start(scale_checkbutton, false, false, 0);

	// Toolbox layout
	options_grid.attach(title_label,
		0, 0, 2, 1);
	options_grid.attach(scale_box,
		0, 1, 2, 1);

	scale_checkbutton.signal_toggled().connect(sigc::mem_fun(*this,&StateRotate_Context::refresh_scale_flag));

	options_grid.set_hexpand();
	options_grid.set_vexpand(false);
	options_grid.set_border_width(GAP*2);
	options_grid.set_row_spacing(GAP);
	options_grid.show_all();

	refresh_tool_options();
	App::dialog_tool_options->present();

	get_work_area()->set_allow_layer_clicks(true);
	get_work_area()->set_duck_dragger(duck_dragger_);

	get_work_area()->set_cursor(Gdk::EXCHANGE);

	App::dock_toolbox->refresh();

	load_settings();
	refresh_scale_flag();
}

void
StateRotate_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_grid);
	App::dialog_tool_options->set_local_name(_("Rotate Tool"));
	App::dialog_tool_options->set_name("rotate");
}

Smach::event_result
StateRotate_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateRotate_Context::event_stop_handler(const Smach::event& /*x*/)
{
	canvas_view_->stop();
	return Smach::RESULT_ACCEPT;
	//throw &state_normal;
	//return Smach::RESULT_OK;
}

StateRotate_Context::~StateRotate_Context()
{
	save_settings();

	get_work_area()->clear_duck_dragger();
	get_work_area()->reset_cursor();

	App::dialog_tool_options->clear();

	App::dock_toolbox->refresh();
}




DuckDrag_Rotate::DuckDrag_Rotate():
	original_angle(),
	original_mag(),
	bad_drag(),
	move_only(),
	use_magnitude(true)
{ }

void
DuckDrag_Rotate::begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& offset)
{
	last_rotate=Vector(1,1);

	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

/*
	if(duckmatic->get_selected_ducks().size()<2)
	{
		bad_drag=true;
		return;
	}
*/
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
		vmin[0]=std::min(vmin[0],p[0]);
		vmin[1]=std::min(vmin[1],p[1]);
		vmax[0]=std::max(vmax[0],p[0]);
		vmax[1]=std::max(vmax[1],p[1]);
		positions.push_back(p);
	}
	center=(vmin+vmax)*0.5;
	if((vmin-vmax).mag()<=EPSILON)
		move_only=true;
	else
		move_only=false;


	synfig::Vector vect(offset-center);
	original_angle=Angle::tan(vect[1],vect[0]);
	original_mag=vect.mag();
}


void
DuckDrag_Rotate::duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector)
{
	if(bad_drag)
		return;

	//std::set<etl::handle<Duck> >::iterator iter;
	synfig::Vector vect(duckmatic->snap_point_to_grid(vector)-center+snap);

	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	if(move_only)
	{
		int i;
		for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		{
			if((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION)continue;

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

	Angle::tan angle(vect[1],vect[0]);
	angle=original_angle-angle;
	Real mag(vect.mag()/original_mag);
	Real sine(Angle::sin(angle).get());
	Real cosine(Angle::cos(angle).get());

	int i;
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		if((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION)continue;

		Vector x(positions[i]-center),p;

		p[0]=cosine*x[0]+sine*x[1];
		p[1]=-sine*x[0]+cosine*x[1];
		if(use_magnitude)p*=mag;
		p+=center;
		(*iter)->set_trans_point(p);
	}
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		if(!((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION))continue;

		Vector x(positions[i]-center),p;

		p[0]=cosine*x[0]+sine*x[1];
		p[1]=-sine*x[0]+cosine*x[1];
		if(use_magnitude)p*=mag;
		p+=center;
		(*iter)->set_trans_point(p);
	}

	last_rotate=vect;
	//snap=Vector(0,0);
}

bool
DuckDrag_Rotate::end_duck_drag(Duckmatic* duckmatic)
{
	if(bad_drag)return false;
	if(move_only)
	{
		synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Move Handle"));
		duckmatic->signal_edited_selected_ducks();
		return true;
	}

	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Rotate Handle"));

	if((last_rotate-Vector(1,1)).mag()>0.0001)
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
