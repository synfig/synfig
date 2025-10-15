/* === S Y N F I G ========================================================= */
/*!	\file layerbrush.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2025 Abdelhadi Wael
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

#include "layerbrush.h"
#include <synfig/rendering/software/surfacesw.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerBrush);
ACTION_SET_NAME(Action::LayerBrush,"LayerBrush");
ACTION_SET_LOCAL_NAME(Action::LayerBrush,N_("Brush Stroke"));
ACTION_SET_TASK(Action::LayerBrush,"brush_stroke");
ACTION_SET_CATEGORY(Action::LayerBrush,Action::CATEGORY_NONE);
ACTION_SET_PRIORITY(Action::LayerBrush,0);
ACTION_SET_VERSION(Action::LayerBrush,"0.0");

#define CHECKPOINT_INTERVAL 20

/* === G L O B A L S ======================================================= */

struct StrokeData {
	Layer_Bitmap::Handle layer;
	std::vector<Action::LayerBrush::StrokePoint> points;
	std::unique_ptr<brushlib::Brush> brush;
	std::unique_ptr<synfig::Surface> checkpoint_surface;
	Point before_tl , before_br;
};

static std::vector<StrokeData> strokes_history;
static std::map<Layer_Bitmap::Handle, Surface> original_layer_surface;

void
Action::LayerBrush::cleanup_history()
{
	strokes_history.clear();
	original_layer_surface.clear();
}
/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerBrush::BrushStroke::BrushStroke():
	prepared(false),
	applied(false),
	stroke_index(-1),
	brush_(new brushlib::Brush()),
	undo_mode(CHECKPOINTING)
{
}

Action::LayerBrush::BrushStroke::~BrushStroke()
{
}

static void
paint_stroke_data(const StrokeData& stroke_data)
{
	if (!stroke_data.brush || stroke_data.points.empty() || !stroke_data.layer)
		return;

	stroke_data.layer->set_param("tl", stroke_data.before_tl);
	stroke_data.layer->set_param("br", stroke_data.before_br);

	Surface tmp;
	{
		rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(stroke_data.layer->rendering_surface);
		if (!lock || !lock->get_surface().is_valid())
			return;
		const Surface &layer_surface = lock->get_surface();
		tmp = synfig::Surface(layer_surface.get_w(), layer_surface.get_h());
		tmp.copy(layer_surface);
	}
	// reset brush
	{
		auto& brush = *stroke_data.brush;
		const auto& first_point = stroke_data.points[0];
		for (int i = 0; i < STATE_COUNT; i++) {
			brush.set_state(i, 0);
		}
		brush.set_state(STATE_X, first_point.x);
		brush.set_state(STATE_Y, first_point.y);
		brush.set_state(STATE_PRESSURE, first_point.pressure);
		brush.set_state(STATE_ACTUAL_X, brush.get_state(STATE_X));
		brush.set_state(STATE_ACTUAL_Y, brush.get_state(STATE_Y));
		brush.set_state(STATE_STROKE, 1.0);
	}
	Point new_tl = stroke_data.before_tl;
	Point new_br = stroke_data.before_br;
	// replay stroke points
	for (const auto &point : stroke_data.points) {
		int old_w = tmp.get_w();
		int old_h = tmp.get_h();

		brushlib::SurfaceWrapper wrapper(&tmp);
		stroke_data.brush->stroke_to(&wrapper, point.x, point.y, point.pressure, 0.0f, 0.0f, point.dtime);

		bool expanded = wrapper.offset_x != 0 || wrapper.offset_y != 0 ||
					wrapper.extra_right > 0 || wrapper.extra_bottom > 0;
		if (expanded) {
			float w = new_br[0] - new_tl[0];
			float h = new_br[1] - new_tl[1];

			if (old_w > 0 && old_h > 0) {
				float units_per_pixel_x = w / old_w;
				float units_per_pixel_y = h / old_h;

				new_tl[0] -= wrapper.offset_x * units_per_pixel_x;
				new_tl[1] -= wrapper.offset_y * units_per_pixel_y;
				new_br[0] += wrapper.extra_right * units_per_pixel_x;
				new_br[1] += wrapper.extra_bottom * units_per_pixel_y;
			}
		}
		float new_x = stroke_data.brush->get_state(STATE_X) + wrapper.offset_x;
		float new_y = stroke_data.brush->get_state(STATE_Y) + wrapper.offset_y;
		stroke_data.brush->set_state(STATE_X, new_x);
		stroke_data.brush->set_state(STATE_Y, new_y);
	}

	stroke_data.layer->set_param("tl" , new_tl);
	stroke_data.layer->set_param("br" , new_br);

	if (stroke_data.layer->rendering_surface && tmp.is_valid()) {
		Surface *surface_copy = new Surface(tmp.get_w(), tmp.get_h());
		surface_copy->copy(tmp);
		stroke_data.layer->rendering_surface = new rendering::SurfaceResource(new rendering::SurfaceSW(*surface_copy, true));
	}
}

void
Action::LayerBrush::BrushStroke::prepare()
{
	new_tl = original_tl = layer->get_param("tl").get(Point());
	new_br = original_br = layer->get_param("br").get(Point());
	switch (undo_mode) {
		case SURFACE_SAVING:
			if (!layer || prepared)
				return;
			if (layer->rendering_surface) {
				rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(layer->rendering_surface);
				if (lock) {
					original_surface = lock->get_surface();
				}
			}
			prepared = true;
			break;
		case REDRAW:
			if (!layer || prepared) return;
			prepared = true;
			break;
		case CHECKPOINTING:
			if (!layer || prepared) return;
			prepared = true;
			break;
	}
}

void
Action::LayerBrush::BrushStroke::apply()
{
	switch (undo_mode) {
		case SURFACE_SAVING:
		{
			if (!prepared || applied || !layer) {
				return;
			}
			if (layer->rendering_surface && final_surface && final_surface->is_valid()) {
				Surface *surface_copy = new Surface(*final_surface);
				layer->rendering_surface = new rendering::SurfaceResource(
					new rendering::SurfaceSW(*surface_copy, true)
				);
				layer->changed();
				applied = true;
			}
			break;
		}
		case REDRAW:
		{
			if (!prepared || applied || !layer || points.empty() || !brush_)
				return;

			// if this is the first stroke on this layer store the original surface
			if (original_layer_surface.find(layer) == original_layer_surface.end()) {
				if (layer->rendering_surface) {
					rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(layer->rendering_surface);
					if (lock && lock->get_surface().is_valid()) {
						original_layer_surface[layer] = lock->get_surface();
					}
				}
			}
			// apply stroke
			if (layer->rendering_surface && final_surface && final_surface->is_valid()) {
				Surface *surface_copy = new Surface(*final_surface);
				layer->rendering_surface = new rendering::SurfaceResource(
					new rendering::SurfaceSW(*surface_copy, true)
				);
				layer->changed();
				applied = true;
			}
			// add to history
			StrokeData stroke_data;
			stroke_data.points = points;
			stroke_data.layer = layer;
			stroke_data.brush = std::move(brush_);
			stroke_data.before_tl = original_tl;
			stroke_data.before_br = original_br;
			strokes_history.push_back(std::move(stroke_data));
			stroke_index = strokes_history.size() - 1;
			break;
		}
		case CHECKPOINTING:
		{
			if (!prepared || applied || !layer || points.empty() || !brush_)
				return;

			// If this is the first stroke on this layer store the original surface
			if (original_layer_surface.find(layer) == original_layer_surface.end()) {
				if (layer->rendering_surface) {
					rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(layer->rendering_surface);
					if (lock && lock->get_surface().is_valid()) {
						original_layer_surface[layer] = lock->get_surface();
					}
				}
			}

			// if we need to checkpoint save the surface
			std::unique_ptr<Surface> temp_checkpoint;
			if (strokes_history.size() % CHECKPOINT_INTERVAL == 0) {
				if(layer->rendering_surface) {
					rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(layer->rendering_surface);
					if (lock && lock->get_surface().is_valid()) {
						temp_checkpoint.reset(new Surface(lock->get_surface()));
					}
				}
			}

			if (layer->rendering_surface && final_surface && final_surface->is_valid()) {
				Surface *surface_copy = new Surface(*final_surface);
				layer->rendering_surface = new rendering::SurfaceResource(
					new rendering::SurfaceSW(*surface_copy, true)
				);
				layer->changed();
				applied = true;
			}

			// add to history
			StrokeData stroke_data;
			stroke_data.layer = layer;
			stroke_data.points = std::move(points);
			stroke_data.brush = std::move(brush_);
			stroke_data.checkpoint_surface = std::move(temp_checkpoint);
			stroke_data.before_tl = original_tl;
			stroke_data.before_br = original_br;
			strokes_history.push_back(std::move(stroke_data));
			stroke_index = strokes_history.size() - 1;
			break;
		}
	}
}

void
Action::LayerBrush::BrushStroke::undo()
{
	layer->set_param("tl", original_tl);
	layer->set_param("br", original_br);
	switch (undo_mode) {
		case SURFACE_SAVING:
		{
			if (!prepared || !applied || !layer || !original_surface.is_valid()) {
				return;
			}
			{
				std::lock_guard<std::mutex> lock(layer->mutex);
				synfig::Surface* surface_copy = new synfig::Surface(original_surface);
				layer->rendering_surface = new rendering::SurfaceResource(
					new rendering::SurfaceSW(*surface_copy, true));
			}
			layer->changed();
			applied = false;
			break;
		}

		case REDRAW:
		{
			if (!applied || !layer || strokes_history.empty() || stroke_index != (int)strokes_history.size() - 1) {
				return;
			}
			brush_ = std::move(strokes_history.back().brush);
			points = std::move(strokes_history.back().points);
			strokes_history.pop_back();

			// restore the original surface
			auto it = original_layer_surface.find(layer);
			if (it != original_layer_surface.end()) {
				const Surface& original_surface = it->second;
				if (layer->rendering_surface && original_surface.is_valid()) {
					rendering::SurfaceResource::LockWrite<rendering::SurfaceSW> lock(layer->rendering_surface);
					if (lock) {
						Surface& dest_surface = lock->get_surface();
						if (dest_surface.get_w() != original_surface.get_w() || dest_surface.get_h() != original_surface.get_h()) {
							dest_surface = Surface(original_surface.get_w(), original_surface.get_h());
						}
						dest_surface.copy(original_surface);
					}
				}
			}

			// Replay strokes
			for (auto& stroke_data : strokes_history) {
				if (stroke_data.layer == layer) {
					paint_stroke_data(stroke_data);
				}
			}

			layer->changed();
			applied = false;
			stroke_index = -1;
			break;
		}

		case CHECKPOINTING:
		{
			if (!applied || !layer || strokes_history.empty() || stroke_index != (int)strokes_history.size() - 1) {
				return;
			}

			brush_ = std::move(strokes_history.back().brush);
			points = std::move(strokes_history.back().points);
			strokes_history.pop_back();

			// find the most recent checkpoint
			int checkpoint_idx = -1;
			for (int i = strokes_history.size() - 1; i >= 0; --i) {
				if (strokes_history[i].layer == layer && strokes_history[i].checkpoint_surface) {
					checkpoint_idx = i;
					break;
				}
			}

			Surface starting_surface;
			int redraw_start = 0;
			if (checkpoint_idx != -1) {
				starting_surface = *strokes_history[checkpoint_idx].checkpoint_surface;
				redraw_start = checkpoint_idx;
			} else {
				auto it = original_layer_surface.find(layer);
				if (it != original_layer_surface.end()) {
					starting_surface = it->second;
				}
			}
			if (layer->rendering_surface && starting_surface.is_valid()) {
				rendering::SurfaceResource::LockWrite<rendering::SurfaceSW> lock(layer->rendering_surface);
				if (lock) {
					lock->get_surface() = starting_surface;
				}
			}

			// replay strokes since last checkpoint
			for (int i = redraw_start; i < strokes_history.size(); ++i) {
				StrokeData& stroke_data = strokes_history[i];
				if (stroke_data.layer == layer) {
					paint_stroke_data(stroke_data);
				}
			}
			layer->changed();
			applied = false;
			stroke_index = -1;
			break;
		}
	}
}

Action::LayerBrush::LayerBrush():
	applied(false)
{
	set_dirty(true);
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
	return stroke.get_layer() && !stroke.get_points().empty() && Action::CanvasSpecific::is_ready();
}

void
Action::LayerBrush::perform()
{
	// store surface before applying stroke
	synfig::Surface before_surface;
	if (stroke.get_layer() && stroke.get_layer()->rendering_surface) {
		rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(stroke.get_layer()->rendering_surface);
		if (lock && lock->get_surface().is_valid())
			before_surface = lock->get_surface();
	}

	// apply stroke
	if (!stroke.is_prepared())
		stroke.prepare();
	stroke.apply();

	// check if surface changed
	bool surfaces_are_equal = false;
	if (stroke.get_layer() && stroke.get_layer()->rendering_surface) {
		rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(stroke.get_layer()->rendering_surface);
		if (lock && lock->get_surface().is_valid() && before_surface.is_valid()) {
			auto after = synfig::rendering::Surface::Handle(new synfig::rendering::SurfaceSW(const_cast<synfig::Surface&>(lock->get_surface()), false));
			auto before = synfig::rendering::Surface::Handle(new synfig::rendering::SurfaceSW(before_surface, false));
			surfaces_are_equal = after->equals_to(before);
		}
	}

	// if no changes detected don't register the action
	if (surfaces_are_equal) {
		stroke.undo();
		applied = true;
		throw Action::Error(Action::Error::TYPE_UNABLE, "");
	}

	if (get_canvas_interface()) {
		get_canvas_interface()->signal_layer_param_changed()(stroke.get_layer(), "rendering_surface");
	}
	if (!applied) stroke.get_layer()->add_surface_modification_id(id);
	applied = !applied;
}

void
Action::LayerBrush::undo()
{
	if (applied) {
		stroke.undo();
		if (get_canvas_interface()) {
			get_canvas_interface()->signal_layer_param_changed()(stroke.get_layer(), "rendering_surface");
		}
		if (applied) stroke.get_layer()->add_surface_modification_id(id);
		applied = !applied;
	}
}