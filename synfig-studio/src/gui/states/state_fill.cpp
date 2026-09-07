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


	CanvasView::Handle get_canvas_view()const{return canvas_view;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view->get_work_area();}


}; // END of class StateFill_Context

/* === G L O B A L S ======================================================= */

StateFill studio::state_fill;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

StateFill::StateFill():
	Smach::state<StateFill_Context>("fill", N_("Fill Tool"))
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
#include <synfigapp/actions/layerpaint.h>
#include <synfig/layers/layer_switch.h>
#include <gui/ducktransform_matrix.h>
bool
build_transform_stack(
	synfig::Canvas::Handle canvas,
	synfig::Layer::Handle layer,
	CanvasView::Handle canvas_view,
	synfig::TransformStack& transform_stack )
{
	int count = 0;
	for (const auto& i : *canvas) {

		if (i == layer)
			return true;

		if (i->active()) {
			synfig::Transform::Handle trans(i->get_transform());
			if (trans) {
				transform_stack.push(trans);
				count++;
			}
		}

		// If this is a paste canvas layer, then we need to
		// descend into it
		if (synfig::Layer_PasteCanvas::Handle layer_pastecanvas = synfig::Layer_PasteCanvas::Handle::cast_dynamic(i)) {
			transform_stack.push_back(
				new studio::Transform_Matrix(
					layer_pastecanvas->get_guid(),
					layer_pastecanvas->get_summary_transformation().get_matrix()
					)
				);

			if (build_transform_stack(layer_pastecanvas->get_sub_canvas(), layer, canvas_view, transform_stack)) {
				return true;
			}
			transform_stack.pop();
		}
	}

	while (count-- > 0) {
		transform_stack.pop();
	}
	return false;
}

Smach::event_result
StateFill_Context::event_workarea_layer_clicked_handler(const Smach::event& x)
{
	const EventLayerClick& event(*reinterpret_cast<const EventLayerClick*>(&x));

	if(!event.layer)
	{
		get_canvas_view()->get_ui_interface()->warning(_("No layer here"));
		return Smach::RESULT_ACCEPT;
	}

	synfig::Layer_Bitmap::Handle layer_bitmap;
	if (auto layer_switch = dynamic_cast<synfig::Layer_Switch*>(event.layer.get())) {
		layer_bitmap = synfig::Layer_Bitmap::Handle::cast_dynamic(layer_switch->get_current_layer());
	} else {
		layer_bitmap = synfig::Layer_Bitmap::Handle::cast_dynamic(event.layer);
	}

	if (layer_bitmap && layer_bitmap->rendering_surface) {
		synfigapp::Action::BitmapLayerFill::Handle action = new synfigapp::Action::BitmapLayerFill();

		action->set_param("layer", synfig::Layer::Handle(layer_bitmap.get()));
		// const synfig::TransformStack& transform(get_work_area()->get_curr_transform_stack());
		synfig::TransformStack transform;
		build_transform_stack(get_canvas(), layer_bitmap, get_canvas_view(), transform);
		synfig::Point pos(transform.unperform(event.pos));
		const synfig::Vector tl = layer_bitmap->get_param("tl").get(synfig::Point());
		const synfig::Vector br = layer_bitmap->get_param("br").get(synfig::Point());
		synfig::warning("pos: %f , %f \t tl: %f , %f \t br: %f , %f", pos[0], pos[1], tl[0], tl[1], br[0], br[1]);
		pos = (pos - tl).divide_coords(br - tl);
		synfig::warning("\tpos: %f , %f", pos[0], pos[1]);
		// float surface_x = ((pos[0] - tl[0]) / (br[0] - tl[0])) * overlay_surface.get_w();
		// float surface_y = ((pos[1] - tl[1]) / (br[1] - tl[1])) * overlay_surface.get_h();
		pos[0] *= layer_bitmap->rendering_surface->get_width();
		pos[1] *= layer_bitmap->rendering_surface->get_height();
		synfig::warning("\t\tpos: %f , %f \t w: %i \t h: %i", pos[0], pos[1], layer_bitmap->rendering_surface->get_width(), layer_bitmap->rendering_surface->get_height());

		action->set_param("point", synfig::ValueBase(pos));
		action->set_param("color", synfig::ValueBase(synfigapp::Main::get_fill_color()));

		action->set_param("canvas", get_canvas());
		action->set_param("canvas_interface", get_canvas_interface());
		get_canvas_interface()->get_instance()->perform_action(action);
		return Smach::RESULT_ACCEPT;
	}


	//synfigapp::Action::Handle action(synfigapp::Action::create("ValueDescSet"));
	synfigapp::ValueDesc value_desc(event.layer,"color");

	if(!get_canvas_interface()->change_value(value_desc,synfig::ValueBase(synfigapp::Main::get_fill_color())))
	{
		get_canvas_view()->get_ui_interface()->warning(_("Unable to set layer color"));
		return Smach::RESULT_ERROR;
	}

	return Smach::RESULT_ACCEPT;
}
