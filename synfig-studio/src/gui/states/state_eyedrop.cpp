/* === S Y N F I G ========================================================= */
/*!	\file state_eyedrop.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <synfig/general.h>

#include "state_eyedrop.h"
#include "state_normal.h"
#include "workarea.h"
#include <synfig/context.h>
#include "app.h"
#include "dialogs/dialog_color.h"
#include "event_mouse.h"
#include "event_layerclick.h"
#include "docks/dock_toolbox.h"
#include "canvasview.h"
#include <synfigapp/main.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
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

void* StateEyedrop::enter_state(studio::CanvasView* machine_context) const
{
	return new StateEyedrop_Context(machine_context);
}

StateEyedrop_Context::StateEyedrop_Context(CanvasView *canvasView):
	canvas_view(canvasView),
	is_working(*canvasView)
{
	synfig::info("Entered Eyedrop State");
	canvas_view->get_work_area()->set_cursor(Gdk::Cursor::create(Gdk::CROSSHAIR));

	App::dock_toolbox->refresh();
}

StateEyedrop_Context::~StateEyedrop_Context()
{
	synfig::info("Left Eyedrop State");
	canvas_view->get_work_area()->reset_cursor();
	App::dock_toolbox->refresh();
}

Smach::event_result
StateEyedrop_Context::event_stop_handler(const Smach::event& /*x*/)
{
	//synfig::info("STATE EYEDROP: Received Stop Event");
	//throw Smach::egress_exception();
	throw &state_normal;
	return Smach::RESULT_OK;
//	canvas_view->get_smach().pop_state();
//	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateEyedrop_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	synfig::info("STATE EYEDROP: Received Refresh Event");
	canvas_view->get_work_area()->queue_render();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateEyedrop_Context::event_workarea_mouse_button_down_handler(const Smach::event& x)
{
	synfig::info("STATE EYEDROP: Received mouse button down Event");
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	if(event.button==BUTTON_LEFT)
	{
		Color color(canvas_view->get_canvas()->get_context(canvas_view->get_context_params()).get_color(event.pos));
		synfigapp::Main::set_outline_color(color);
		studio::App::dialog_color->set_color(color);
		return Smach::RESULT_ACCEPT;
	}
	return Smach::RESULT_OK;
}
