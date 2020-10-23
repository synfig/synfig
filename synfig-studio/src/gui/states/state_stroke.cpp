/* === S Y N F I G ========================================================= */
/*!	\file state_stroke.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "state_stroke.h"

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/event_mouse.h>
#include <gui/exception_guard.h>
#include <gui/workarea.h>

#include <synfig/valuenodes/valuenode_dynamiclist.h>

#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StateStroke studio::state_stroke;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateStroke_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	Duckmatic::Push duckmatic_push;

	etl::smart_ptr<std::list<synfig::Point> > stroke_data;

	etl::smart_ptr<std::list<synfig::Real> > width_data;

	Gdk::ModifierType modifier;

public:

	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_mouse_up_handler(const Smach::event& x);

	Smach::event_result event_mouse_draw_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	StateStroke_Context(CanvasView* canvas_view);

	~StateStroke_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

};	// END of class StateStroke_Context


/* === M E T H O D S ======================================================= */

StateStroke::StateStroke():
	Smach::state<StateStroke_Context>("stroke")
{
	insert(event_def(EVENT_STOP,&StateStroke_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateStroke_Context::event_refresh_handler));
//	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateStroke_Context::event_mouse_down_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateStroke_Context::event_mouse_up_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateStroke_Context::event_mouse_draw_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateStroke_Context::event_refresh_tool_options));
}

StateStroke::~StateStroke()
{
}

void* StateStroke::enter_state(studio::CanvasView* machine_context) const
{
	return new StateStroke_Context(machine_context);
}

StateStroke_Context::StateStroke_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	duckmatic_push(get_work_area()),
	modifier()
{
	width_data.spawn();
	stroke_data.spawn();

	get_work_area()->add_stroke(stroke_data, synfigapp::Main::get_outline_color());
}

StateStroke_Context::~StateStroke_Context()
{
	duckmatic_push.restore();

	App::dock_toolbox->refresh();

	SYNFIG_EXCEPTION_GUARD_BEGIN()
	// Send the stroke data to whatever previously called this state.
	if(stroke_data->size()>=2)
		get_canvas_view()->get_smach().process_event(EventStroke(stroke_data,width_data,modifier));
	SYNFIG_EXCEPTION_GUARD_END_NO_RETURN()
}

Smach::event_result
StateStroke_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateStroke_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw Smach::pop_exception();
}

Smach::event_result
StateStroke_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateStroke_Context::event_mouse_up_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			modifier=event.modifier;
			throw Smach::pop_exception();
		}

	default:
		return Smach::RESULT_OK;
	}
}

Smach::event_result
StateStroke_Context::event_mouse_draw_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			stroke_data->push_back(event.pos);
			width_data->push_back(event.pressure);
			get_work_area()->queue_draw();
			return Smach::RESULT_ACCEPT;
		}

	default:
		return Smach::RESULT_OK;
	}
}
