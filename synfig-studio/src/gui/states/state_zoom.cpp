/* === S Y N F I G ========================================================= */
/*!	\file state_zoom.cpp
**	\brief Zoom Tool Implementation File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
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

#include "state_zoom.h"

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/event_mouse.h>
#include <gui/states/state_normal.h>
#include <gui/workarea.h>

#include <synfig/vector.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */
StateZoom studio::state_zoom;

const float ZOOMFACTOR = 1.25f;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateZoom_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	WorkArea::PushState push_state;

	Point p1,p2;

public:

	//events
	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_mouse_click_handler(const Smach::event& x);

	//constructor destructor
	StateZoom_Context(CanvasView* canvas_view);
	~StateZoom_Context();

	//Canvas interaction
	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	//void zoom(const Point& p1, const Point& p2);

};	// END of class StateZoom_Context

/* === M E T H O D S ======================================================= */

StateZoom::StateZoom():
	Smach::state<StateZoom_Context>("zoom")
{
	insert(event_def(EVENT_STOP,&StateZoom_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateZoom_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateZoom_Context::event_mouse_click_handler));
	//insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateZoom_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_BOX,&StateZoom_Context::event_mouse_click_handler));
	//insert(event_def(EVENT_WORKAREA_BUTTON_CLICK,&StateZoom_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateZoom_Context::event_mouse_click_handler));
}

StateZoom::~StateZoom()
{
}

void* StateZoom::enter_state(studio::CanvasView* machine_context) const
{
	return new StateZoom_Context(machine_context);
}

StateZoom_Context::StateZoom_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	push_state(*get_work_area())
{
	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// Hide all ducks
	get_work_area()->set_type_mask(Duck::TYPE_NONE);
	get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);

	get_work_area()->set_cursor(Gdk::CROSSHAIR);

	App::dock_toolbox->refresh();
}

StateZoom_Context::~StateZoom_Context()
{
	// Refresh the work area
	get_work_area()->queue_draw();

	App::dock_toolbox->refresh();
}

Smach::event_result
StateZoom_Context::event_stop_handler(const Smach::event& /*x*/)
{
	//throw Smach::egress_exception();
	throw &state_normal;
	return Smach::RESULT_OK;
}

Smach::event_result
StateZoom_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateZoom_Context::event_mouse_click_handler(const Smach::event& x)
{
	if(x.key==EVENT_WORKAREA_BOX)
	{
		const EventBox& event(*reinterpret_cast<const EventBox*>(&x));

		if(event.button==BUTTON_LEFT)
		{
			Point tl = get_work_area()->get_window_tl();
			Point br = get_work_area()->get_window_br();
			Vector	window_span = br - tl, window_middle = (br+tl)/2;
			Vector	box_span = event.p2 - event.p1, box_middle = (event.p1+event.p2)/2;
			Point newpos;
			float zoom;

			if(event.modifier & Gdk::CONTROL_MASK) //zoom out...
			{
				if (window_span[0] == 0 || window_span[1] == 0) zoom = 1;
				else zoom = std::max(std::fabs(box_span[0]/window_span[0]), std::fabs(box_span[1]/window_span[1]));

				// focus_point is -1 times the real position for some reason...
				// center the window so the old contents fill the drawn box
				newpos = -((window_middle - box_middle)/zoom + window_middle);
			}
			else				// zoom in
			{
				if (box_span[0] == 0 || box_span[1] == 0) zoom = 1;
				else zoom = std::min(std::fabs(window_span[0]/box_span[0]), std::fabs(window_span[1]/box_span[1]));

				// center the window at the center of the box
				newpos = -(-get_work_area()->get_focus_point() + (box_middle - window_middle));
			}

			get_work_area()->set_focus_point(newpos);
			get_work_area()->set_zoom(get_work_area()->get_zoom()*zoom);

			return Smach::RESULT_ACCEPT;
		}
	}

	if(x.key==EVENT_WORKAREA_MOUSE_BUTTON_UP)
	{
		const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

		if(event.button==BUTTON_LEFT)
		{
			Point evpos;

			//make the event pos be in the same space...
			//   The weird ass inverted center normalized space...
			{
				const Point realcenter = (get_work_area()->get_window_tl() + get_work_area()->get_window_br())/2;
				evpos = -(event.pos - realcenter) + get_work_area()->get_focus_point();
			}

			/*	Zooming:
				focus point must zoom about the point evpos...

				trans about an origin not 0:
				p' = A(p - o) + o
			*/

			Vector v = get_work_area()->get_focus_point() - evpos;

			if(event.modifier & Gdk::CONTROL_MASK) //zoom out...
			{
				v*=ZOOMFACTOR;
				//get_work_area()->zoom_out();
				get_work_area()->set_focus_point(evpos + v);
				get_work_area()->set_zoom(get_work_area()->get_zoom()/ZOOMFACTOR);
			}else //zoom in
			{
				v/=ZOOMFACTOR;
				//get_work_area()->zoom_in();
				get_work_area()->set_focus_point(evpos + v);
				get_work_area()->set_zoom(get_work_area()->get_zoom()*ZOOMFACTOR);
			}

			return Smach::RESULT_ACCEPT;
		}
	}

	return Smach::RESULT_OK;
}
