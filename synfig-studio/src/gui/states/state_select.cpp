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
	Gtk::Label title_label;
	//i think not needed anymore
	bool pressed = false;

	Gtk::Label prioritize_groups_label;
	Gtk::CheckButton prioritize_groups_checkbutton;
	Gtk::Box prioritize_groups_box;

public:

	explicit StateSelect_Context(CanvasView* canvas_view);

	~StateSelect_Context();

	bool get_lock_animation_flag()const
	{
		return get_canvas_view()
			&& get_canvas_view()->get_work_area()
			&& get_canvas_view()->get_work_area()->get_lock_animation_mode();
	}
	void set_lock_animation_flag(bool x)
	{
		if ( get_canvas_view()
		  && get_canvas_view()->get_work_area()
		  && get_canvas_view()->get_work_area()->get_lock_animation_mode() != x )
		{
			get_canvas_view()->get_work_area()->set_lock_animation_mode(x);
			get_canvas_view()->get_work_area()->queue_draw();
		}
	}

	void load_settings();
	void save_settings();

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
//	Smach::event_result event_mouse_motion_handler(const Smach::event& x);
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
//	insert(event_def(EVENT_WORKAREA_MOUSE_MOTION,		&StateSelect_Context::event_mouse_motion_handler));
//	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,	&StateSelect_Context::event_mouse_motion_handler));
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

	//should words be capitalized like this ?
	prioritize_groups_label.set_label(_("Priotirize Group Selection"));
	prioritize_groups_label.set_hexpand();
	prioritize_groups_label.set_halign(Gtk::ALIGN_START);
	prioritize_groups_label.set_valign(Gtk::ALIGN_CENTER);
	prioritize_groups_box.pack_start(prioritize_groups_label, true, true, 0);
	prioritize_groups_box.pack_start(prioritize_groups_checkbutton, false, false, 0);

	prioritize_groups_checkbutton.signal_toggled().connect(sigc::mem_fun(*this,&StateSelect_Context::save_settings));

	// Toolbox layout
	options_grid.attach(title_label,
		0, 0, 1, 1);
	options_grid.attach(prioritize_groups_box,
		0, 1, 2, 1);


	options_grid.set_border_width(GAP*2);
	options_grid.set_row_spacing(GAP);
	options_grid.set_margin_bottom(0);
	options_grid.show_all();

	refresh_tool_options();

	get_work_area()->set_allow_layer_clicks(true);
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

DuckDrag_Select::DuckDrag_Select(){

}

void
DuckDrag_Select::begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& offset)
{
	is_moving = false;
	last_move=Vector(1,1);

	const DuckList selected_movement_ducks(duckmatic->get_selected_movement_ducks());
	DuckList::const_iterator iter;

	bad_drag=false;

//		drag_offset=duckmatic->find_duck(offset)->get_trans_point();
		drag_offset=offset;
		//snap=drag_offset-duckmatic->snap_point_to_grid(drag_offset);
		//snap=offset-drag_offset_;
		snap=Vector(0,0);

	// Calculate center
	Point vmin(100000000,100000000);
	Point vmax(-100000000,-100000000);
	//std::set<etl::handle<Duck> >::iterator iter;
	positions.clear();
	int i;
	for(i=0,iter=selected_movement_ducks.begin();iter!=selected_movement_ducks.end();++iter,i++)
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

bool DuckDrag_Select::end_duck_drag(Duckmatic *duckmatic)
{
	if(bad_drag)return false;

	//synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Rotate Ducks"));

	if(is_moving)
	{
		duckmatic->signal_edited_selected_movement_ducks();
		return true;
	}
	else
	{
		//probably dont need this idk
		duckmatic->signal_user_click_selected_ducks(0);
		return false;
	}
}

void DuckDrag_Select::duck_drag(Duckmatic *duckmatic, const synfig::Vector &vector)
{
	if (!duckmatic) return;

	if(bad_drag)
		return;

	// this is quick-hack mostly, so need to check if nothing broken
	//if (!move_only && !scale && !rotate) return; // nothing to do

	//Override axis lock set in workarea when holding down the shift key
	if (!move_only && (scale || rotate))
		duckmatic->set_axis_lock(false);

	synfig::Vector vect;
	if (move_only || (!scale && !rotate))
		vect= duckmatic->snap_point_to_grid(vector)-drag_offset+snap;
	else
		vect= duckmatic->snap_point_to_grid(vector)-center+snap;

	last_move=vect;

	const DuckList selected_movement_ducks(duckmatic->get_selected_movement_ducks());
	DuckList::const_iterator iter;

	Time time(duckmatic->get_time());
	int i;
	if( move_only || (!scale && !rotate) )
	{
		for(i=0,iter=selected_movement_ducks.begin();iter!=selected_movement_ducks.end();++iter,i++)
		{
			if((*iter)->get_type()==Duck::TYPE_VERTEX || (*iter)->get_type()==Duck::TYPE_POSITION)
				(*iter)->set_trans_point(positions[i]+vect, time);
		}
		for(i=0,iter=selected_movement_ducks.begin();iter!=selected_movement_ducks.end();++iter,i++)
		{
			if((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION)
				(*iter)->set_trans_point(positions[i]+vect, time);
		}
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

		for(i=0,iter=selected_movement_ducks.begin();iter!=selected_movement_ducks.end();++iter,i++)
		{
			if((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION)continue;

			Vector x(positions[i]-center),p;

			p[0]=cosine*x[0]+sine*x[1];
			p[1]=-sine*x[0]+cosine*x[1];
			if(scale)p*=mag;
			p+=center;
			(*iter)->set_trans_point(p, time);
		}
		for(i=0,iter=selected_movement_ducks.begin();iter!=selected_movement_ducks.end();++iter,i++)
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
			if(std::fabs(drag_offset[0]-center[0])>EPSILON)
				vect[0]/=drag_offset[0]-center[0];
			else
				vect[0]=1;
			if(std::fabs(drag_offset[1]-center[1])>EPSILON)
				vect[1]/=drag_offset[1]-center[1];
			else
				vect[1]=1;
			}
		else
		{
			Real amount;
			if((drag_offset-center).mag() < EPSILON)
				amount = 1;
			else
				amount = vect.mag()/(drag_offset-center).mag();

			vect[0]=vect[1]=amount;
		}

		if(vect[0]<EPSILON && vect[0]>-EPSILON)
			vect[0]=1;
		if(vect[1]<EPSILON && vect[1]>-EPSILON)
			vect[1]=1;

		for(i=0,iter=selected_movement_ducks.begin();iter!=selected_movement_ducks.end();++iter,i++)
		{
			if(((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION))continue;

			Vector p(positions[i]-center);

			p[0]*=vect[0];
			p[1]*=vect[1];
			p+=center;
			(*iter)->set_trans_point(p, time);
		}
		for(i=0,iter=selected_movement_ducks.begin();iter!=selected_movement_ducks.end();++iter,i++)
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

	if((last_move-Vector(1,1)).mag()>0.0001)
		is_moving = true;

	if (is_moving)
		duckmatic->signal_edited_selected_movement_ducks(true);

	// then patch up the tangents for the vertices we've moved
	duckmatic->update_ducks();

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

//Smach::event_result
//StateSelect_Context::event_mouse_motion_handler(const Smach::event& x)
//{
//	//synfig::info("STATE SELECT: Received mouse button motion Event");
//	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

//	return Smach::RESULT_OK;
//}

Smach::event_result
StateSelect_Context::event_mouse_release_handler(const Smach::event& x){
	return Smach::RESULT_OK;
}

Smach::event_result
StateSelect_Context::event_layer_click(const Smach::event& x)
{
	const EventLayerClick& event(*reinterpret_cast<const EventLayerClick*>(&x));

	if(event.layer)
	{
		//synfig::info("STATE SELECT: Received layer click Event, \"%s\"",event.layer->get_name().c_str());
		 pressed = true;
	}
	else
	{
		//synfig::info("STATE SELECT: Received layer click Event with an empty layer.");
	}

	switch(event.button)
	{
	case BUTTON_LEFT:
		if(!(event.modifier&Gdk::CONTROL_MASK))
			canvas_view_->get_selection_manager()->clear_selected_layers();
		if(event.layer)
		{
			std::list<Layer::Handle> layer_list(canvas_view_->get_selection_manager()->get_selected_layers());
			std::set<Layer::Handle> layers(layer_list.begin(),layer_list.end());
			if(layers.count(event.layer))
			{
				layers.erase(event.layer);
				layer_list=std::list<Layer::Handle>(layers.begin(),layers.end());
				canvas_view_->get_selection_manager()->clear_selected_layers();
				canvas_view_->get_selection_manager()->set_selected_layers(layer_list);
			}
			else
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
