/* === S Y N F I G ========================================================= */
/*!	\file state_fill.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <gui/states/state_fill.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/event_layerclick.h>
#include <gui/localization.h>
#include <gui/states/state_normal.h>
#include <gui/workarea.h>

#include <synfig/general.h>

#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateFill_Context
{
	CanvasView *canvas_view;
	CanvasView::IsWorking is_working;

	WorkArea::PushState push_state;

public:
	StateFill_Context(CanvasView *canvas_view);
	~StateFill_Context();

	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_workarea_layer_clicked_handler(const Smach::event& x);


	etl::handle<CanvasView> get_canvas_view()const{return canvas_view;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view->get_canvas();}
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

void* StateFill::enter_state(studio::CanvasView* machine_context) const
{
	return new StateFill_Context(machine_context);
}

StateFill_Context::StateFill_Context(CanvasView *canvasView):
	canvas_view(canvasView),
	is_working(*canvasView),
	push_state(*get_work_area())
{
	synfig::info("Entered Fill State");

	// Disable duck and bezier clicking
	get_work_area()->set_allow_duck_clicks(false);
	get_work_area()->set_allow_bezier_clicks(false);

	// Hide all ducks
	get_work_area()->set_type_mask(Duck::TYPE_NONE);
	get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);

	canvas_view->get_work_area()->set_cursor(Gdk::CROSSHAIR);

	App::dock_toolbox->refresh();
}

StateFill_Context::~StateFill_Context()
{
	synfig::info("Left Fill State");
	canvas_view->get_work_area()->reset_cursor();
	App::dock_toolbox->refresh();
}

Smach::event_result
StateFill_Context::event_stop_handler(const Smach::event& /*x*/)
{
	synfig::info("STATE FILL: Received Stop Event");
	//throw Smach::egress_exception();
	throw &state_normal;
	return Smach::RESULT_OK;
//	canvas_view->get_smach().pop_state();
//	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateFill_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	synfig::info("STATE FILL: Received Refresh Event");
	canvas_view->get_work_area()->queue_render();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateFill_Context::event_workarea_layer_clicked_handler(const Smach::event& x)
{
	synfig::info("STATE FILL: Received layer clicked Event");
	const EventLayerClick& event(*reinterpret_cast<const EventLayerClick*>(&x));

	if(!event.layer)
	{
		get_canvas_view()->get_ui_interface()->warning(_("No layer here"));
		return Smach::RESULT_ACCEPT;
	}


	//synfigapp::Action::Handle action(synfigapp::Action::create("ValueDescSet"));
	synfigapp::ValueDesc value_desc(event.layer,"color");

	if(!get_canvas_interface()->change_value(value_desc,synfig::ValueBase(synfigapp::Main::get_fill_color())))
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
	//	synfig::error("LayerParamConnect didn't like \"param\"");
	if(!action->set_param("new_value",ValueBase(synfigapp::Main::get_fill_color())))
		synfig::error("LayerParamConnect didn't like \"fill_color\"");

	if(!get_canvas_interface()->get_instance()->perform_action(action))
	{
		get_canvas_view()->get_ui_interface()->warning(_("Unable to set layer color"));
		return Smach::RESULT_ERROR;
	}
	get_canvas_view()->get_ui_interface()->task(_("Idle"));
	*/
	return Smach::RESULT_ACCEPT;
}
