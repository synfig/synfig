/* === S I N F G =========================================================== */
/*!	\file state_fill.cpp
**	\brief Template File
**
**	$Id: state_fill.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include "state_fill.h"
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

class studio::StateFill_Context
{
	CanvasView *canvas_view;
	CanvasView::IsWorking is_working;

public:
	StateFill_Context(CanvasView *canvas_view);
	~StateFill_Context();
	
	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_workarea_layer_clicked_handler(const Smach::event& x);


	etl::handle<CanvasView> get_canvas_view()const{return canvas_view;}
	etl::handle<sinfgapp::CanvasInterface> get_canvas_interface()const{return canvas_view->canvas_interface();}
	sinfg::Canvas::Handle get_canvas()const{return canvas_view->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view->get_work_area();}


}; // END of class StateFill_Context

/* === G L O B A L S ======================================================= */

StateFill studio::state_fill;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

StateFill::StateFill():
	Smach::state<StateFill_Context>("fill")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StateFill_Context::event_stop_handler));
	insert(event_def(EVENT_STOP,&StateFill_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateFill_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_LAYER_CLICKED,&StateFill_Context::event_workarea_layer_clicked_handler));
}	

StateFill::~StateFill()
{
}

StateFill_Context::StateFill_Context(CanvasView *canvas_view):
	canvas_view(canvas_view),
	is_working(*canvas_view)
{
	sinfg::info("Enterted Fill State");
	canvas_view->work_area->set_cursor(Gdk::CROSSHAIR);
	
	App::toolbox->refresh();
}

StateFill_Context::~StateFill_Context()
{
	sinfg::info("Left Fill State");
	canvas_view->work_area->reset_cursor();
	App::toolbox->refresh();
}

Smach::event_result
StateFill_Context::event_stop_handler(const Smach::event& x)
{
	sinfg::info("STATE FILL: Received Stop Event");
	throw Smach::egress_exception();
//	canvas_view->get_smach().pop_state();
//	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateFill_Context::event_refresh_handler(const Smach::event& x)
{
	sinfg::info("STATE FILL: Received Refresh Event");
	canvas_view->work_area->queue_render_preview();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateFill_Context::event_workarea_layer_clicked_handler(const Smach::event& x)
{
	sinfg::info("STATE FILL: Received layer clicked Event");
	const EventLayerClick& event(*reinterpret_cast<const EventLayerClick*>(&x));

	if(!event.layer)
	{
		get_canvas_view()->get_ui_interface()->warning(_("No layer here"));
		return Smach::RESULT_ACCEPT;
	}

	
	//sinfgapp::Action::Handle action(sinfgapp::Action::create("value_desc_set"));
	sinfgapp::ValueDesc value_desc(event.layer,"color");

	if(!get_canvas_interface()->change_value(value_desc,ValueBase(sinfgapp::Main::get_foreground_color())))
	{
		get_canvas_view()->get_ui_interface()->warning(_("Unable to set layer color"));
		return Smach::RESULT_ERROR;
	}
	/*
	assert(action);
	
	action->set_param("canvas",get_canvas());			
	action->set_param("canvas_interface",get_canvas_interface());			
	action->set_param("value_desc",value_desc);
	action->set_param("time",get_canvas_interface()->get_time());
	//action->set_param("layer",event.layer);			
	//if(!action->set_param("param",String("color")))
	//	sinfg::error("LayerParamConnect didn't like \"param\"");
	if(!action->set_param("new_value",ValueBase(sinfgapp::Main::get_foreground_color())))
		sinfg::error("LayerParamConnect didn't like \"foreground_color\"");
	
	if(!get_canvas_interface()->get_instance()->perform_action(action))
	{
		get_canvas_view()->get_ui_interface()->warning(_("Unable to set layer color"));
		return Smach::RESULT_ERROR;
	}
	get_canvas_view()->get_ui_interface()->task(_("Idle"));
	*/
	return Smach::RESULT_ACCEPT;
}
