/* === S I N F G =========================================================== */
/*!	\file state_eyedrop.cpp
**	\brief Template File
**
**	$Id: state_eyedrop.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include "state_eyedrop.h"
#include "workarea.h"
#include <sinfg/context.h>
#include "app.h"
#include "dialog_color.h"
#include "event_mouse.h"
#include "event_layerclick.h"
#include "toolbox.h"
#include "canvasview.h"
#include <sinfgapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateEyedrop_Context
{
	CanvasView *canvas_view;
	CanvasView::IsWorking is_working;

public:
	StateEyedrop_Context(CanvasView *canvas_view);
	~StateEyedrop_Context();
	
	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_workarea_mouse_button_down_handler(const Smach::event& x);

}; // END of class StateEyedrop_Context

/* === G L O B A L S ======================================================= */

StateEyedrop studio::state_eyedrop;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

StateEyedrop::StateEyedrop():
	Smach::state<StateEyedrop_Context>("eyedrop")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StateEyedrop_Context::event_stop_handler));
	insert(event_def(EVENT_STOP,&StateEyedrop_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateEyedrop_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateEyedrop_Context::event_workarea_mouse_button_down_handler));
}	

StateEyedrop::~StateEyedrop()
{
}

StateEyedrop_Context::StateEyedrop_Context(CanvasView *canvas_view):
	canvas_view(canvas_view),
	is_working(*canvas_view)
{
	sinfg::info("Enterted Eyedrop State");
	canvas_view->work_area->set_cursor(Gdk::Cursor(Gdk::CROSSHAIR));
	
	App::toolbox->refresh();
}

StateEyedrop_Context::~StateEyedrop_Context()
{
	sinfg::info("Left Eyedrop State");
	canvas_view->work_area->reset_cursor();
	App::toolbox->refresh();
}

Smach::event_result
StateEyedrop_Context::event_stop_handler(const Smach::event& x)
{
	sinfg::info("STATE EYEDROP: Received Stop Event");
	throw Smach::egress_exception();
//	canvas_view->get_smach().pop_state();
//	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateEyedrop_Context::event_refresh_handler(const Smach::event& x)
{
	sinfg::info("STATE EYEDROP: Received Refresh Event");
	canvas_view->work_area->queue_render_preview();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateEyedrop_Context::event_workarea_mouse_button_down_handler(const Smach::event& x)
{
	sinfg::info("STATE EYEDROP: Received mouse button down Event");
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	if(event.button==BUTTON_LEFT)
	{
		Color color(canvas_view->get_canvas()->get_context().get_color(event.pos));
		sinfgapp::Main::set_foreground_color(color);
		studio::App::dialog_color->set_color(color);
		return Smach::RESULT_ACCEPT;
	}
	return Smach::RESULT_OK;
}
