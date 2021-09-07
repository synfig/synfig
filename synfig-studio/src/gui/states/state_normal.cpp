/* === S Y N F I G ========================================================= */
/*!	\file state_normal.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
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

#include <gui/states/state_normal.h>

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

using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef EPSILON
#define EPSILON	0.0000001
#endif

const int GAP = 3;

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

	bool is_moving;

public:
	CanvasView* canvas_view_;
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
	CanvasView* canvas_view_;

	etl::handle<DuckDrag_Combo> duck_dragger_;

	Gtk::Grid options_grid;
	Gtk::Label title_label;

	bool ctrl_pressed;
	bool alt_pressed;
	bool shift_pressed;
	bool space_pressed;

	void set_ctrl_pressed(bool value);
	void set_alt_pressed(bool value);
	void set_shift_pressed(bool value);
	void set_space_pressed(bool value);

public:

	void refresh_cursor();

	bool get_rotate_flag()const { if(duck_dragger_) return duck_dragger_->rotate; else return false; }
	void set_rotate_flag(bool x) { if(duck_dragger_ && x!=duck_dragger_->rotate)
		                               {duck_dragger_->rotate=x; refresh_cursor();} }

	bool get_scale_flag()const { if(duck_dragger_) return duck_dragger_->scale; else return false; }
	void set_scale_flag(bool x) { if(duck_dragger_ && x!=duck_dragger_->scale)
		                              {duck_dragger_->scale=x; refresh_cursor();} }

	bool get_constrain_flag()const { if(duck_dragger_) return duck_dragger_->constrain; else return false; }
	void set_constrain_flag(bool x) { if(duck_dragger_ && x!=duck_dragger_->constrain)
		                                  {duck_dragger_->constrain=x; refresh_cursor();} }

	bool get_alternative_flag()const
	{
		return get_canvas_view()
			&& get_canvas_view()->get_work_area()
			&& get_canvas_view()->get_work_area()->get_alternative_mode();
	}
	void set_alternative_flag(bool x)
	{
		if ( get_canvas_view()
		  && get_canvas_view()->get_work_area()
		  && get_canvas_view()->get_work_area()->get_alternative_mode() != x )
		{
			get_canvas_view()->get_work_area()->set_alternative_mode(x);
			get_canvas_view()->get_work_area()->queue_draw();
			refresh_cursor();
		}
	}

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
			refresh_cursor();
		}
	}

	StateNormal_Context(CanvasView* canvas_view);

	~StateNormal_Context();

	CanvasView* get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_refresh_ducks_handler(const Smach::event& x);
	Smach::event_result event_undo_handler(const Smach::event& x);
	Smach::event_result event_redo_handler(const Smach::event& x);
	Smach::event_result event_mouse_button_down_handler(const Smach::event& x);
	Smach::event_result event_multiple_ducks_clicked_handler(const Smach::event& x);
	Smach::event_result event_mouse_motion_handler(const Smach::event& x);
	Smach::event_result event_key_down_handler(const Smach::event& x);
	Smach::event_result event_key_up_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	void refresh_tool_options();
	Smach::event_result event_layer_click(const Smach::event& x);


};	// END of class StateNormal_Context

/* === M E T H O D S ======================================================= */

StateNormal::StateNormal():
	Smach::state<StateNormal_Context>("normal")
{
	insert(event_def(EVENT_STOP,&StateNormal_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateNormal_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateNormal_Context::event_refresh_ducks_handler));
	insert(event_def(EVENT_UNDO,&StateNormal_Context::event_undo_handler));
	insert(event_def(EVENT_REDO,&StateNormal_Context::event_redo_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateNormal_Context::event_mouse_button_down_handler));
	insert(event_def(EVENT_WORKAREA_MULTIPLE_DUCKS_CLICKED,&StateNormal_Context::event_multiple_ducks_clicked_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateNormal_Context::event_refresh_tool_options));
	insert(event_def(EVENT_WORKAREA_MOUSE_MOTION,		&StateNormal_Context::event_mouse_motion_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,	&StateNormal_Context::event_mouse_motion_handler));
	insert(event_def(EVENT_WORKAREA_KEY_DOWN,&StateNormal_Context::event_key_down_handler));
	insert(event_def(EVENT_WORKAREA_KEY_UP,&StateNormal_Context::event_key_up_handler));
	insert(event_def(EVENT_WORKAREA_LAYER_CLICKED,&StateNormal_Context::event_layer_click));

}

StateNormal::~StateNormal()
{
}

void* StateNormal::enter_state(studio::CanvasView* machine_context) const
{
	return new StateNormal_Context(machine_context);
}

void StateNormal_Context::refresh_cursor()
{
	// Check the current state and return when applicable
	synfig::String sname;
	sname=get_canvas_view()->get_smach().get_state_name();
	if (sname!="normal")
			return;

	// Change the cursor based on key flags
	if(get_rotate_flag() && !get_scale_flag())
	{
	    //!TODO Do not change the cursor in WorkArea::DragMode mode, but actually not stable enough to catch
	    //! real DRAGBOX mode (go to DRAGNONE too quick)
		get_work_area()->set_cursor(Gdk::EXCHANGE);
		return;
	}
	if(!get_rotate_flag() && get_scale_flag())
	{
		get_work_area()->set_cursor(Gdk::SIZING);
		return;
	}
	if(get_rotate_flag() && get_scale_flag())
	{
		get_work_area()->set_cursor(Gdk::CROSSHAIR);
		return;
	}
	if(get_lock_animation_flag())
	{
		get_work_area()->set_cursor(Gdk::DRAFT_LARGE);
		return;
	}

	// Default cursor for Transform tool
	get_work_area()->set_cursor(Gdk::ARROW);

}

StateNormal_Context::StateNormal_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	duck_dragger_(new DuckDrag_Combo()),
	ctrl_pressed(),
	alt_pressed(),
	shift_pressed(),
	space_pressed(false)
{
	duck_dragger_->canvas_view_=get_canvas_view();

	// Toolbox widgets
	title_label.set_label(_("Transform Tool"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_hexpand();
	title_label.set_halign(Gtk::ALIGN_START);
	title_label.set_valign(Gtk::ALIGN_CENTER);

	// Toolbox layout
	options_grid.attach(title_label,
		0, 0, 1, 1);
	options_grid.attach(*manage(new Gtk::Label(_("Ctrl to rotate"), Gtk::ALIGN_START)),
		0, 1, 1, 1);
	options_grid.attach(*manage(new Gtk::Label(_("Alt to scale"), Gtk::ALIGN_START)),
		0, 2, 1, 1);
	options_grid.attach(*manage(new Gtk::Label(_("Shift to constrain"), Gtk::ALIGN_START)),
		0, 3, 1, 1);

	options_grid.set_border_width(GAP*2);
	options_grid.set_row_spacing(GAP);
	options_grid.set_margin_bottom(0);
	options_grid.show_all();

	refresh_tool_options();

	get_work_area()->set_allow_layer_clicks(true);
	get_work_area()->set_duck_dragger(duck_dragger_);

	App::dock_toolbox->refresh();
}

void
StateNormal_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_grid);
	App::dialog_tool_options->set_local_name(_("Transform Tool"));
	App::dialog_tool_options->set_name("normal");
}



StateNormal_Context::~StateNormal_Context()
{
	get_work_area()->clear_duck_dragger();
	get_work_area()->reset_cursor();

	App::dialog_tool_options->clear();

	App::dock_toolbox->refresh();
}

DuckDrag_Combo::DuckDrag_Combo():
	original_angle(),
	original_mag(),
	bad_drag(),
	move_only(),
	is_moving(false),
	canvas_view_(NULL),
	scale(false),
	rotate(false),
	constrain(false) // Lock aspect for scale
{ }

void
DuckDrag_Combo::begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& offset)
{
	is_moving = false;
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
DuckDrag_Combo::duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector)
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

	if((last_move-Vector(1,1)).mag()>0.0001)
		is_moving = true;

	if (is_moving)
		duckmatic->signal_edited_selected_ducks(true);

	// then patch up the tangents for the vertices we've moved
	duckmatic->update_ducks();
}

bool
DuckDrag_Combo::end_duck_drag(Duckmatic* duckmatic)
{
	if(bad_drag)return false;

	//synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Rotate Ducks"));

	if(is_moving)
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

Smach::event_result
StateNormal_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNormal_Context::event_stop_handler(const Smach::event& /*x*/)
{
	// synfig::info("STATE NORMAL: Received Stop Event");
	canvas_view_->stop();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNormal_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	// synfig::info("STATE NORMAL: Received Refresh Event");
	canvas_view_->rebuild_tables();
	canvas_view_->get_work_area()->queue_render();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNormal_Context::event_refresh_ducks_handler(const Smach::event& /*x*/)
{
	// synfig::info("STATE NORMAL: Received Refresh Ducks");
	canvas_view_->queue_rebuild_ducks();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNormal_Context::event_undo_handler(const Smach::event& /*x*/)
{
	// synfig::info("STATE NORMAL: Received Undo Event");
	canvas_view_->get_instance()->undo();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNormal_Context::event_redo_handler(const Smach::event& /*x*/)
{
	// synfig::info("STATE NORMAL: Received Redo Event");
	canvas_view_->get_instance()->redo();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNormal_Context::event_mouse_button_down_handler(const Smach::event& x)
{
	// synfig::info("STATE NORMAL: Received mouse button down Event");

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

Smach::event_result
StateNormal_Context::event_mouse_motion_handler(const Smach::event& x)
{
	// synfig::info("STATE NORMAL: Received mouse button down Event");

	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	set_ctrl_pressed(event.modifier&GDK_CONTROL_MASK);
	set_alt_pressed(event.modifier&GDK_MOD1_MASK);
	set_shift_pressed(event.modifier&GDK_SHIFT_MASK);

	return Smach::RESULT_OK;
}

void
StateNormal_Context::set_ctrl_pressed(bool value)
{
	if (ctrl_pressed == value) return;
	ctrl_pressed = value;

	if (ctrl_pressed)
	{
		if (get_canvas_view()->get_work_area()->get_selected_ducks().size() <= 1
		 /* && get_canvas_view()->get_work_area()->get_selected_duck()->get_value_desc().get_value_type() == synfig::type_transformation */ )
		{
			set_rotate_flag(false);
			set_alternative_flag(true);
		}
		else
		{
			set_rotate_flag(true);
			set_alternative_flag(false);
		}
	}
	else
	{
		set_alternative_flag(false);
		set_rotate_flag(false);
	}
}

void
StateNormal_Context::set_alt_pressed(bool value)
{
	if (alt_pressed == value) return;
	alt_pressed = value;
	set_scale_flag(alt_pressed);
}

void
StateNormal_Context::set_shift_pressed(bool value)
{
	if (shift_pressed == value) return;
	shift_pressed = value;
	set_constrain_flag(shift_pressed);
}

void
StateNormal_Context::set_space_pressed(bool value)
{
	if (space_pressed == value) return;
	space_pressed = value;
	set_lock_animation_flag(space_pressed);
}

Smach::event_result
StateNormal_Context::event_key_down_handler(const Smach::event& x)
{
	// event.modifier yet not set when ctrl (or alt or shift)
	// key pressed event handled. So we need to check this keys manually.
	// We may encountred some cosmetic problems with mouse-cursor image
	// if user will redefine modifier keys.
	// Anyway processing of keys Ctrl+Right, Ctrl+Left etc will works fine.
	// see 'xmodmap' command
	const EventKeyboard& event(*reinterpret_cast<const EventKeyboard*>(&x));
	switch(event.keyval)
	{
	case GDK_KEY_Control_L:
	case GDK_KEY_Control_R:
		set_ctrl_pressed(true);
		break;
	case GDK_KEY_Alt_L:
	case GDK_KEY_Alt_R:
	case GDK_KEY_Meta_L:
		set_alt_pressed(true);
		break;
	case GDK_KEY_Shift_L:
	case GDK_KEY_Shift_R:
		set_shift_pressed(true);
		break;
	case GDK_KEY_space:
		set_space_pressed(true);
		break;
	default:
		set_ctrl_pressed(event.modifier&GDK_CONTROL_MASK);
		set_alt_pressed(event.modifier&GDK_MOD1_MASK);
		set_shift_pressed(event.modifier&GDK_SHIFT_MASK);
		break;
	}
	return Smach::RESULT_REJECT;
}

Smach::event_result
StateNormal_Context::event_key_up_handler(const Smach::event& x)
{
	// see event_key_down_handler for possible problems
	const EventKeyboard& event(*reinterpret_cast<const EventKeyboard*>(&x));
	switch(event.keyval)
	{
	case GDK_KEY_Control_L:
	case GDK_KEY_Control_R:
		set_ctrl_pressed(false);
		break;
	case GDK_KEY_Alt_L:
	case GDK_KEY_Alt_R:
	case GDK_KEY_Meta_L:
		set_alt_pressed(false);
		break;
	case GDK_KEY_Shift_L:
	case GDK_KEY_Shift_R:
		set_shift_pressed(false);
		break;
	case GDK_KEY_space:
		set_space_pressed(false);
		break;
	default:
		break;
	}
	return Smach::RESULT_REJECT;
}

Smach::event_result
StateNormal_Context::event_layer_click(const Smach::event& x)
{
	const EventLayerClick& event(*reinterpret_cast<const EventLayerClick*>(&x));

	if(event.layer)
	{
		// synfig::info("STATE NORMAL: Received layer click Event, \"%s\"",event.layer->get_name().c_str());
	}
	else
	{
		// synfig::info("STATE NORMAL: Received layer click Event with an empty layer.");
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

Smach::event_result
StateNormal_Context::event_multiple_ducks_clicked_handler(const Smach::event& x)
{
	// synfig::info("STATE NORMAL: Received multiple duck click event");

	//const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	std::list<synfigapp::ValueDesc> value_desc_list;

	// Create a list of value_descs associated with selection
	const DuckList selected_ducks(get_work_area()->get_selected_ducks());
	DuckList::const_iterator iter;
	for(iter=selected_ducks.begin();iter!=selected_ducks.end();++iter)
	{
		synfigapp::ValueDesc value_desc((*iter)->get_value_desc());

		if(!value_desc.is_valid())
			continue;
		value_desc_list.push_back(value_desc);
	}

	Gtk::Menu *menu=manage(new Gtk::Menu());
	menu->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), menu));

	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	canvas_view_->get_instance()->make_param_menu(
			menu,
			canvas_view_->get_canvas(),
			value_desc_list,
			event.duck ? event.duck->get_value_desc() : synfigapp::ValueDesc()
		);

	menu->popup(3,gtk_get_current_event_time());

	return Smach::RESULT_ACCEPT;
}


