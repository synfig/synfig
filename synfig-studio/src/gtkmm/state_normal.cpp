/* === S Y N F I G ========================================================= */
/*!	\file state_normal.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <synfig/valuenode_animated.h>
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_const.h>
#include <synfig/valuenode_dynamiclist.h>
#include <synfigapp/action_system.h>

#include "state_normal.h"
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
#include <synfig/angle.h>
#include <synfigapp/main.h>

#include "general.h"
#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef EPSILON
#define EPSILON	0.0000001
#endif

/* === G L O B A L S ======================================================= */

StateNormal studio::state_normal;

/* === C L A S S E S & S T R U C T S ======================================= */

class DuckDrag_Combo : public DuckDrag_Base
{
	synfig::Vector last_move;
	synfig::Vector drag_offset;
	synfig::Vector center;
	synfig::Vector snap;

	synfig::Angle original_angle;
	synfig::Real original_mag;

	std::vector<synfig::Vector> last_;
	std::vector<synfig::Vector> positions;


	bool bad_drag;
	bool move_only;

public:
	etl::handle<CanvasView> canvas_view_;
	bool scale;
	bool rotate;
	bool constrain;
	DuckDrag_Combo();
	void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector);

	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
};


class studio::StateNormal_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;

	synfigapp::Settings& settings;

	sigc::connection keypress_connect;
	sigc::connection keyrelease_connect;

	etl::handle<DuckDrag_Combo> duck_dragger_;

	Gtk::Table options_table;

	Gtk::CheckButton checkbutton_rotate;
	Gtk::CheckButton checkbutton_scale;
	Gtk::CheckButton checkbutton_constrain;

public:

	bool get_rotate_flag()const { return checkbutton_rotate.get_active(); }
	void set_rotate_flag(bool x) { checkbutton_rotate.set_active(x); refresh_rotate_flag(); }
	void refresh_rotate_flag() { if(duck_dragger_)duck_dragger_->rotate=get_rotate_flag(); }

	bool get_scale_flag()const { return checkbutton_scale.get_active(); }
	void set_scale_flag(bool x) { checkbutton_scale.set_active(x); refresh_scale_flag(); }
	void refresh_scale_flag() { if(duck_dragger_)duck_dragger_->scale=get_scale_flag(); }

	bool get_constrain_flag()const { return checkbutton_constrain.get_active(); }
	void set_constrain_flag(bool x) { checkbutton_constrain.set_active(x); refresh_constrain_flag(); }
	void refresh_constrain_flag() { if(duck_dragger_)duck_dragger_->constrain=get_constrain_flag(); }

	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	void refresh_tool_options();

	StateNormal_Context(CanvasView* canvas_view);

	~StateNormal_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	void load_settings();
	void save_settings();

	bool key_pressed(GdkEventKey *event);
	bool key_released(GdkEventKey *event);

};	// END of class StateNormal_Context

/* === M E T H O D S ======================================================= */

StateNormal::StateNormal():
	Smach::state<StateNormal_Context>("normal")
{
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateNormal_Context::event_refresh_tool_options));
}

StateNormal::~StateNormal()
{
}

void
StateNormal_Context::load_settings()
{
	String value;

	if(settings.get_value("normal.rotate",value) && value=="1")
		set_rotate_flag(true);
	else
		set_rotate_flag(false);

	if(settings.get_value("normal.scale",value) && value=="1")
		set_scale_flag(true);
	else
		set_scale_flag(false);

	if(settings.get_value("normal.constrain",value) && value=="1")
		set_constrain_flag(true);
	else
		set_constrain_flag(false);

}

void
StateNormal_Context::save_settings()
{
	settings.set_value("normal.rotate",get_rotate_flag()?"1":"0");
	settings.set_value("normal.scale",get_scale_flag()?"1":"0");
	settings.set_value("normal.constrain",get_constrain_flag()?"1":"0");
}

StateNormal_Context::StateNormal_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	duck_dragger_(new DuckDrag_Combo()),
	checkbutton_rotate(_("Rotate (Ctrl)")),
	checkbutton_scale(_("Scale (Alt)")),
	checkbutton_constrain(_("Constrain (Shift)"))
{
	duck_dragger_->canvas_view_=get_canvas_view();

	// Set up the tool options dialog
	options_table.attach(*manage(new Gtk::Label(_("Normal Tool"))),	0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_rotate,							0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_scale,							0, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_constrain,							0, 2, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	checkbutton_rotate.signal_toggled().connect(sigc::mem_fun(*this,&StateNormal_Context::refresh_rotate_flag));
	checkbutton_scale.signal_toggled().connect(sigc::mem_fun(*this,&StateNormal_Context::refresh_scale_flag));
	checkbutton_constrain.signal_toggled().connect(sigc::mem_fun(*this,&StateNormal_Context::refresh_constrain_flag));


	options_table.show_all();
	refresh_tool_options();
	//App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->present();

	get_work_area()->set_allow_layer_clicks(true);
	get_work_area()->set_duck_dragger(duck_dragger_);

	keypress_connect=get_work_area()->signal_key_press_event().connect(sigc::mem_fun(*this,&StateNormal_Context::key_pressed),false);
	keyrelease_connect=get_work_area()->signal_key_release_event().connect(sigc::mem_fun(*this,&StateNormal_Context::key_released),false);

//	get_canvas_view()->work_area->set_cursor(Gdk::CROSSHAIR);
	get_canvas_view()->work_area->reset_cursor();

	App::toolbox->refresh();

	load_settings();
	refresh_scale_flag();
}

bool
StateNormal_Context::key_pressed(GdkEventKey *event)
{
	switch(event->keyval)
	{
		case GDK_Control_L:
		case GDK_Control_R:
			set_rotate_flag(true);
			break;
		case GDK_Alt_L:
		case GDK_Alt_R:
			set_scale_flag(true);
			break;
		case GDK_Shift_L:
		case GDK_Shift_R:
			set_constrain_flag(true);
			break;
		default:
			break;
	}
	return false; //Pass on the event to other handlers, just in case
}

bool
StateNormal_Context::key_released(GdkEventKey *event)
{
	switch(event->keyval)
	{
		case GDK_Control_L:
		case GDK_Control_R:
			set_rotate_flag(false);
			break;
		case GDK_Alt_L:
		case GDK_Alt_R:
			set_scale_flag(false);
			break;
		case GDK_Shift_L:
		case GDK_Shift_R:
			set_constrain_flag(false);
			break;
		default:
			break;
	}
	return false; //Pass on the event to other handlers
}

void
StateNormal_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Normal Tool"));
	App::dialog_tool_options->set_name("normal");
}



StateNormal_Context::~StateNormal_Context()
{
	save_settings();

	get_work_area()->clear_duck_dragger();
	get_canvas_view()->work_area->reset_cursor();

	keypress_connect.disconnect();
	keyrelease_connect.disconnect();

	App::dialog_tool_options->clear();

	App::toolbox->refresh();
}


Smach::event_result
StateNormal_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

DuckDrag_Combo::DuckDrag_Combo():
	scale(false),
	rotate(false),
	constrain(false) // Lock aspect for scale; smooth move for translate
{
}

void
DuckDrag_Combo::begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& offset)
{
	last_move=Vector(1,1);

	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	bad_drag=false;

		drag_offset=duckmatic->find_duck(offset)->get_trans_point();

		//snap=drag_offset-duckmatic->snap_point_to_grid(drag_offset);
		//snap=offset-drag_offset_;
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
DuckDrag_Combo::duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector)
{
	if (!duckmatic) return;

	if(bad_drag)
		return;

	//Override axis lock set in workarea when holding down the shift key
	if (!move_only && (scale || rotate))
		duckmatic->set_axis_lock(false);

	synfig::Vector vect;
	if (move_only || (!scale && !rotate))
		vect= duckmatic->snap_point_to_grid(vector)-drag_offset+snap;
	else
		vect= duckmatic->snap_point_to_grid(vector)-center+snap;

	last_move=vect;

	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	Time time(duckmatic->get_time());

	int i;
	if( move_only || (!scale && !rotate) )
	{
		for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		{
			if((*iter)->get_type()==Duck::TYPE_VERTEX || (*iter)->get_type()==Duck::TYPE_POSITION)
				(*iter)->set_trans_point(positions[i]+vect, time);
		}
		for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		{
			if((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION)
				(*iter)->set_trans_point(positions[i]+vect, time);
		}
		DuckList duck_list(duckmatic->get_duck_list());
		for (iter=duck_list.begin(); iter!=duck_list.end(); ++iter)
		{
			if ((*iter)->get_type() == Duck::TYPE_TANGENT || (*iter)->get_type() == Duck::TYPE_WIDTH)
			{
				(*iter)->update(time);
			}
		}
		return;
	}

	if (rotate)
	{
		Angle::deg angle(Angle::tan(vect[1],vect[0]));
		angle=original_angle-angle;
		if (constrain)
		{
			float degrees = angle.get()/15;
			angle= Angle::deg (degrees>0?std::floor(degrees)*15:std::ceil(degrees)*15);
		}
		Real mag(vect.mag()/original_mag);
		Real sine(Angle::sin(angle).get());
		Real cosine(Angle::cos(angle).get());

		for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		{
			if((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION)continue;

			Vector x(positions[i]-center),p;

			p[0]=cosine*x[0]+sine*x[1];
			p[1]=-sine*x[0]+cosine*x[1];
			if(scale)p*=mag;
			p+=center;
			(*iter)->set_trans_point(p, time);
		}
		for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		{
			if(!((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION))continue;

			Vector x(positions[i]-center),p;

			p[0]=cosine*x[0]+sine*x[1];
			p[1]=-sine*x[0]+cosine*x[1];
			if(scale)p*=mag;
			p+=center;
			(*iter)->set_trans_point(p, time);
		}
	} else if (scale)
	{
		if(!constrain)
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

		for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		{
			if(((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION))continue;

			Vector p(positions[i]-center);

			p[0]*=vect[0];
			p[1]*=vect[1];
			p+=center;
			(*iter)->set_trans_point(p, time);
		}
		for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		{
			if(!((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION))continue;

			Vector p(positions[i]-center);

			p[0]*=vect[0];
			p[1]*=vect[1];
			p+=center;
			(*iter)->set_trans_point(p, time);
		}
	}
	last_move=vect;
}

bool
DuckDrag_Combo::end_duck_drag(Duckmatic* duckmatic)
{
	if(bad_drag)return false;

	//synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Rotate Ducks"));

	if((last_move-Vector(1,1)).mag()>0.0001)
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
