/* === S Y N F I G ========================================================= */
/*!	\file state_mirror.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <gui/states/state_mirror.h>

#include <gtkmm/radiobutton.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/event_mouse.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/states/state_normal.h>
#include <gui/workarea.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

enum Axis {
	AXIS_X,
	AXIS_Y
} ;

const int GAP = 3;

/* === G L O B A L S ======================================================= */

StateMirror studio::state_mirror;

/* === C L A S S E S & S T R U C T S ======================================= */

class DuckDrag_Mirror : public DuckDrag_Base
{
	synfig::Vector center;

	std::vector<synfig::Vector> positions;

public:
	Axis axis;

	DuckDrag_Mirror();
	void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector);
};

class studio::StateMirror_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	etl::handle<DuckDrag_Mirror> duck_dragger_;

	Gtk::Grid options_grid;
	Gtk::Label title_label;

	sigc::connection keypress_connect;
	sigc::connection keyrelease_connect;
	bool shift_is_pressed;
	Gtk::RadioButton::Group radiobutton_group;
	Gtk::RadioButton radiobutton_axis_x;
	Gtk::RadioButton radiobutton_axis_y;

public:

	Axis get_axis()const { return radiobutton_axis_x.get_active()?AXIS_X:AXIS_Y; }
	void set_axis(Axis a)
	{
		if(a==AXIS_X)
			radiobutton_axis_x.set_active(true);
		else
			radiobutton_axis_y.set_active(true);

		duck_dragger_->axis=get_axis();
	}

	void update_axes()
	{
		duck_dragger_->axis=get_axis();
		get_work_area()->set_cursor(get_axis() == AXIS_X?Gdk::SB_H_DOUBLE_ARROW:Gdk::SB_V_DOUBLE_ARROW);
	}

	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	Smach::event_result event_mouse_motion_handler(const Smach::event& x);

	void refresh_tool_options();

	StateMirror_Context(CanvasView* canvas_view);

	~StateMirror_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	bool key_press_event(GdkEventKey *event);
	bool key_release_event(GdkEventKey *event);


};	// END of class StateMirror_Context

/* === M E T H O D S ======================================================= */

StateMirror::StateMirror():
	Smach::state<StateMirror_Context>("mirror")
{
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateMirror_Context::event_refresh_tool_options));
	insert(event_def(EVENT_STOP,&StateMirror_Context::event_stop_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_MOTION,		&StateMirror_Context::event_mouse_motion_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,	&StateMirror_Context::event_mouse_motion_handler));
}

StateMirror::~StateMirror()
{
}

void* StateMirror::enter_state(studio::CanvasView* machine_context) const
{
	return new StateMirror_Context(machine_context);
}

StateMirror_Context::StateMirror_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	duck_dragger_(new DuckDrag_Mirror()),
	radiobutton_axis_x(radiobutton_group,_("Horizontal")),
	radiobutton_axis_y(radiobutton_group,_("Vertical"))
{
	// Toolbox widgets
	title_label.set_label(_("Mirror Tool"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_hexpand();
	title_label.set_halign(Gtk::ALIGN_START);
	title_label.set_valign(Gtk::ALIGN_CENTER);

	// Toolbox layout
	options_grid.attach(title_label,
		0, 0, 2, 1);
	options_grid.attach(radiobutton_axis_x,
		0, 1, 1, 1);
	options_grid.attach(radiobutton_axis_y,
		0, 2, 1, 1);
	options_grid.attach(*manage(new Gtk::Label(_("(Shift key toggles axis)"))),
		0, 3, 2, 1);

	options_grid.set_border_width(GAP*2);
	options_grid.set_row_spacing(GAP);
	options_grid.show_all();

	radiobutton_axis_x.signal_toggled().connect(sigc::mem_fun(*this,&StateMirror_Context::update_axes));
	radiobutton_axis_y.signal_toggled().connect(sigc::mem_fun(*this,&StateMirror_Context::update_axes));
	shift_is_pressed=false;
	keypress_connect=get_work_area()->signal_key_press_event().connect(sigc::mem_fun(*this,&StateMirror_Context::key_press_event),false);
	keyrelease_connect=get_work_area()->signal_key_release_event().connect(sigc::mem_fun(*this,&StateMirror_Context::key_release_event),false);

	refresh_tool_options();
	App::dialog_tool_options->present();

	get_work_area()->set_allow_layer_clicks(true);
	get_work_area()->set_duck_dragger(duck_dragger_);

	get_work_area()->set_cursor(Gdk::SB_H_DOUBLE_ARROW);
//	get_work_area()->reset_cursor();

	App::dock_toolbox->refresh();

	set_axis(AXIS_Y);
}

bool
StateMirror_Context::key_press_event(GdkEventKey *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	if (event->keyval==GDK_KEY_Shift_L || event->keyval==GDK_KEY_Shift_R)
	{
		if (shift_is_pressed) return false;
		shift_is_pressed=true;
		set_axis(get_axis()==AXIS_X ? AXIS_Y:AXIS_X);
		update_axes();
	}

	return false; //Pass on the event to other handlers, just in case
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool
StateMirror_Context::key_release_event(GdkEventKey *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	if (event->keyval==GDK_KEY_Shift_L || event->keyval==GDK_KEY_Shift_R )
	{
		if (!shift_is_pressed) return false;
		shift_is_pressed = false;
		set_axis(get_axis()==AXIS_X ? AXIS_Y:AXIS_X);
		update_axes();
	}

	return false; //Pass on the event to other handlers, just in case
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

void
StateMirror_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_grid);
	App::dialog_tool_options->set_local_name(_("Mirror Tool"));
	App::dialog_tool_options->set_name("mirror");
}

Smach::event_result
StateMirror_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateMirror_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw &state_normal;
	return Smach::RESULT_OK;
}

Smach::event_result
StateMirror_Context::event_mouse_motion_handler(const Smach::event& x)
{
	// synfig::info("STATE NORMAL: Received mouse button down Event");

	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	bool shift_state = event.modifier&GDK_SHIFT_MASK;
	if (shift_state != shift_is_pressed)
	{
		shift_is_pressed = !shift_is_pressed;
		set_axis(get_axis()==AXIS_X ? AXIS_Y:AXIS_X);
	}

	return Smach::RESULT_OK;
}

StateMirror_Context::~StateMirror_Context()
{
	get_work_area()->clear_duck_dragger();
	get_work_area()->reset_cursor();

	keypress_connect.disconnect();
	keyrelease_connect.disconnect();

	App::dialog_tool_options->clear();

	App::dock_toolbox->refresh();
}

DuckDrag_Mirror::DuckDrag_Mirror():
	axis(AXIS_X)
{
}

#ifndef EPSILON
#define EPSILON	0.0000001
#endif

void
DuckDrag_Mirror::begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& /*offset*/)
{
	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	positions.clear();
	int i;
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		Point p((*iter)->get_trans_point());
		positions.push_back(p);
	}

}

void
DuckDrag_Mirror::duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector)
{
	center=vector;
	int i;

	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	//Time time(duckmatic->get_time());

	// do the Vertex and Position ducks first
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		if ((*iter)->get_type() == Duck::TYPE_VERTEX ||
			(*iter)->get_type() == Duck::TYPE_POSITION)
		{
			Vector p(positions[i]);

			if		(axis==AXIS_X) p[0] = -(p[0]-center[0]) + center[0];
			else if	(axis==AXIS_Y) p[1] = -(p[1]-center[1]) + center[1];

			(*iter)->set_trans_point(p);
		}

	// then do the other ducks
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		if ((*iter)->get_type() != Duck::TYPE_VERTEX &&
			(*iter)->get_type() != Duck::TYPE_POSITION)
		{
			// we don't need to mirror radius ducks - they're one-dimensional
			if ((*iter)->is_radius())
				continue;

			Vector p(positions[i]);

			if		(axis==AXIS_X) p[0] = -(p[0]-center[0]) + center[0];
			else if	(axis==AXIS_Y) p[1] = -(p[1]-center[1]) + center[1];

			(*iter)->set_trans_point(p);
		}
}

bool
DuckDrag_Mirror::end_duck_drag(Duckmatic* duckmatic)
{
	duckmatic->signal_edited_selected_ducks();
	return true;
}
