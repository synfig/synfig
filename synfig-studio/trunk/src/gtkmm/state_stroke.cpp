/* === S I N F G =========================================================== */
/*!	\file state_stroke.cpp
**	\brief Template File
**
**	$Id: state_stroke.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "state_stroke.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"
#include <sinfg/valuenode_bline.h>
#include <ETL/hermite>
#include <ETL/calculus>
#include <utility>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "toolbox.h"
#include <sinfgapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
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
	
	etl::smart_ptr<std::list<sinfg::Point> > stroke_data;

	etl::smart_ptr<std::list<sinfg::Real> > width_data;

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
	etl::handle<sinfgapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	sinfg::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
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


StateStroke_Context::StateStroke_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	duckmatic_push(get_work_area())
{
	width_data.spawn();
	stroke_data.spawn();

	get_work_area()->add_stroke(stroke_data, sinfgapp::Main::get_foreground_color());

	sinfg::info("Now Scribbling...");	
}

StateStroke_Context::~StateStroke_Context()
{
	duckmatic_push.restore();
	
	App::toolbox->refresh();
	sinfg::info("No longer scribbling");	

	// Send the stroke data to whatever previously called this state.
	if(stroke_data->size()>=2)
		get_canvas_view()->get_smach().process_event(EventStroke(stroke_data,width_data,modifier));
}

Smach::event_result
StateStroke_Context::event_refresh_tool_options(const Smach::event& x)
{
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateStroke_Context::event_stop_handler(const Smach::event& x)
{
	throw Smach::pop_exception();
}

Smach::event_result
StateStroke_Context::event_refresh_handler(const Smach::event& x)
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
	
	case BUTTON_RIGHT: // Intercept the right-button click to short-circut the pop-up menu
		return Smach::RESULT_ACCEPT;
	
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
	
	case BUTTON_RIGHT: // Intercept the right-button click to short-circut the pop-up menu
		return Smach::RESULT_ACCEPT;
	
	default:	
		return Smach::RESULT_OK;
	}
}
