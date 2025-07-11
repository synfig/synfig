/* === S Y N F I G ========================================================= */
/*!	\file layerbrush.cpp
**	\brief Brush stroke action for StateBrush2 with live preview
**
**	\legal
**	......... ... 2024
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

#include <synfig/general.h>
#include <algorithm> // for std::max, std::min

#include "layerbrush.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>
#include <synfigapp/instance.h>

#include <synfig/rendering/software/surfacesw.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerBrush);
ACTION_SET_NAME(Action::LayerBrush,"LayerBrush");
ACTION_SET_LOCAL_NAME(Action::LayerBrush,N_("Brush Stroke"));
ACTION_SET_TASK(Action::LayerBrush,"brush_stroke");
ACTION_SET_CATEGORY(Action::LayerBrush,Action::CATEGORY_NONE);
ACTION_SET_PRIORITY(Action::LayerBrush,0);
ACTION_SET_VERSION(Action::LayerBrush,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerBrush::BrushStroke::BrushStroke():
	prepared(false),
	applied(false)
{
}

Action::LayerBrush::BrushStroke::~BrushStroke()
{
}

void
Action::LayerBrush::BrushStroke::reset_brush(const StrokePoint& point, int surface_w, int surface_h)
{
	for (int i = 0; i < STATE_COUNT; i++) {
		brush_.set_state(i, 0);
	}
	float surface_x = point.pos[0];
	float surface_y = point.pos[1];
	brush_.set_state(STATE_X, surface_x);
	brush_.set_state(STATE_Y, surface_y);
	brush_.set_state(STATE_PRESSURE, point.pressure);
	brush_.set_state(STATE_ACTUAL_X, surface_x);
	brush_.set_state(STATE_ACTUAL_Y, surface_y);
	brush_.set_state(STATE_STROKE, 1.0);
}

void
Action::LayerBrush::BrushStroke::render_stroke_to_surface(synfig::Surface& surface)
{
	if (points.empty()) {
		return;
	}

	if (!surface.is_valid() || surface.get_w() <= 0 || surface.get_h() <= 0) {
		return;
	}

	brushlib::SurfaceWrapper wrapper(&surface);
	reset_brush(points.front(), surface.get_w(), surface.get_h());

	for (size_t i = 0; i < points.size(); ++i) {
		const auto& point = points[i];

		float surface_x = point.pos[0];
		float surface_y = point.pos[1];
		double dtime;
		if (i == 0) {
			dtime = 0.00001;
		} else {
			dtime = (point.timestamp - points[i-1].timestamp).as_double();
			if (dtime < 0.00001) dtime = 0.00001;
		}

		brush_.stroke_to(&wrapper, surface_x, surface_y, point.pressure, 0.0f, 0.0f, dtime);
		if (wrapper.offset_x != 0 || wrapper.offset_y != 0) {
			float x = brush_.get_state(STATE_X) + wrapper.offset_x;
			float y = brush_.get_state(STATE_Y) + wrapper.offset_y;
			brush_.set_state(STATE_X, x);
			brush_.set_state(STATE_Y, y);
			brush_.set_state(STATE_ACTUAL_X, x);
			brush_.set_state(STATE_ACTUAL_Y, y);
		}
	}

}

void
Action::LayerBrush::BrushStroke::prepare()
{
	if (!layer || prepared) return;
	prepared = true;
}

void
Action::LayerBrush::BrushStroke::apply()
{
	if (!prepared || applied || !layer) {
		return;
	}

	if (layer->rendering_surface) {
		rendering::SurfaceResource::LockWrite<rendering::SurfaceSW> lock(layer->rendering_surface);
		if (lock && lock->get_surface().is_valid()) {
			Surface& surface = lock->get_surface();
			render_stroke_to_surface(surface);
			layer->changed();
			applied = true;
		}
	}
}

void
Action::LayerBrush::BrushStroke::undo()
{
}

Action::LayerBrush::LayerBrush():
	applied(false)
{
}

Action::ParamVocab
Action::LayerBrush::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer", Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to paint on"))
	);

	return ret;
}

bool
Action::LayerBrush::is_candidate(const ParamList& x)
{
	// Check if we have a layer parameter and it's a bitmap layer
	for (ParamList::const_iterator i = x.begin(); i != x.end(); ++i) {
		if (i->first == "layer" && i->second.get_type() == Param::TYPE_LAYER) {
			Layer::Handle layer = i->second.get_layer();
			if (Layer_Bitmap::Handle::cast_dynamic(layer)) {
				return true;
			}
		}
	}
	return false;
}

bool
Action::LayerBrush::set_param(const synfig::String& name, const Action::Param& param)
{
	if (name == "layer" && param.get_type() == Param::TYPE_LAYER) {
		Layer_Bitmap::Handle bitmap_layer = Layer_Bitmap::Handle::cast_dynamic(param.get_layer());
		if (bitmap_layer) {
			stroke.set_layer(bitmap_layer);
			return true;
		}
		return false;
	}

	return Action::CanvasSpecific::set_param(name, param);
}

bool
Action::LayerBrush::is_ready() const
{
	if (!stroke.get_layer()) {
		return false;
	}
	if (stroke.get_points().empty()) {
		return false;
	}
	
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerBrush::perform()
{
	if (!stroke.is_prepared()) {
		stroke.prepare();
	}
	stroke.apply();
	applied = true;
	if (get_canvas_interface()) {
		get_canvas_interface()->signal_layer_param_changed()(stroke.get_layer(), "rendering_surface");
	}
}

void
Action::LayerBrush::undo()
{
}