/* === S Y N F I G ========================================================= */
/*!	\file layerbrush.cpp
**	\brief Template File
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerBrush);
ACTION_SET_NAME(Action::LayerBrush,"LayerBrush");
ACTION_SET_LOCAL_NAME(Action::LayerBrush,N_("Brush Stroke"));
ACTION_SET_TASK(Action::LayerBrush,"brush_stroke");
ACTION_SET_CATEGORY(Action::LayerBrush,Action::CATEGORY_NONE);
ACTION_SET_PRIORITY(Action::LayerBrush,0);
ACTION_SET_VERSION(Action::LayerBrush,"0.0");

#define CHECKPOINT_INTERVAL 100

/* === G L O B A L S ======================================================= */

struct StrokeData {
	Layer_Bitmap::Handle layer;
	std::vector<Action::LayerBrush::StrokePoint> points;
	std::unique_ptr<brushlib::Brush> brush;
	std::unique_ptr<synfig::Surface> checkpoint_surface;
};

static std::vector<StrokeData> strokes_history;
static std::map<Layer_Bitmap::Handle, Surface> original_layer_surface;

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

void
Action::LayerBrush::BrushStroke::reset_brush(const StrokePoint& point)
{
	for (int i = 0; i < STATE_COUNT; i++) {
		brush_->set_state(i, 0);
	}
	brush_->set_state(STATE_X, point.x);
	brush_->set_state(STATE_Y, point.y);
	brush_->set_state(STATE_PRESSURE, point.pressure);
	brush_->set_state(STATE_ACTUAL_X, brush_->get_state(STATE_X));
	brush_->set_state(STATE_ACTUAL_Y, brush_->get_state(STATE_Y));
	brush_->set_state(STATE_STROKE, 1.0);
}

void
Action::LayerBrush::BrushStroke::paint_stroke(synfig::Surface& surface)
{
	if (!brush_ || points.empty() || !surface.is_valid()) return;

	brushlib::SurfaceWrapper wrapper(&surface);
	reset_brush(points[0]);
	brush_->stroke_to(&wrapper, points[0].x, points[0].y, points[0].pressure, 0.0f, 0.0f, 0.0f);
	for (auto &point : points) {
		brush_->stroke_to(&wrapper, point.x, point.y, point.pressure, 0.0f, 0.0f, point.dtime);
	}
}

void
Action::LayerBrush::BrushStroke::prepare()
{
	switch (undo_mode) {
		case SURFACE_SAVING:
			if (!layer || prepared) return;
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
			if (layer->rendering_surface) {
				rendering::SurfaceResource::LockWrite<rendering::SurfaceSW> lock(layer->rendering_surface);
				if (lock && lock->get_surface().is_valid()) {
					Surface& surface = lock->get_surface();
					paint_stroke(surface);
					layer->changed();
					applied = true;
				}
			}
			break;
		}
		case REDRAW:
		{
			if (!prepared || applied || !layer || points.empty() || !brush_) return;

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
			if (layer->rendering_surface) {
				rendering::SurfaceResource::LockWrite<rendering::SurfaceSW> lock(layer->rendering_surface);
				if (lock && lock->get_surface().is_valid()) {
					Surface& surface = lock->get_surface();
					paint_stroke(surface);
					layer->changed();
					applied = true;
				}
			}
			// add to history
			StrokeData stroke_data;
			stroke_data.points = points;
			stroke_data.layer = layer;
			stroke_data.brush = std::move(brush_);
			strokes_history.push_back(std::move(stroke_data));
			stroke_index = strokes_history.size() - 1;
			break;
		}
		case CHECKPOINTING:
		{
			if (!prepared || applied || !layer || points.empty() || !brush_) return;

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

			// apply the stroke
			if (layer->rendering_surface) {
				rendering::SurfaceResource::LockWrite<rendering::SurfaceSW> lock(layer->rendering_surface);
				if (lock && lock->get_surface().is_valid()) {
					paint_stroke(lock->get_surface());
					layer->changed();
					applied = true;
				}
			}

			// add to history
			StrokeData stroke_data;
			stroke_data.layer = layer;
			stroke_data.points = std::move(points);
			stroke_data.brush = std::move(brush_);
			stroke_data.checkpoint_surface = std::move(temp_checkpoint);

			strokes_history.push_back(std::move(stroke_data));
			stroke_index = strokes_history.size() - 1;
			break;
		}
	}
}

void
Action::LayerBrush::BrushStroke::undo()
{
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

			Surface restored_surface = original_layer_surface.find(layer)->second;
			// replay strokes that belong to this layer
			for (auto& stroke_data : strokes_history) {
				if (stroke_data.layer == layer) {
					if (!stroke_data.brush || stroke_data.points.empty()) {
						continue;
					}
					brushlib::SurfaceWrapper wrapper(&restored_surface);
					for (int i = 0; i < STATE_COUNT; i++) stroke_data.brush->set_state(i, 0);

					stroke_data.brush->set_state(STATE_X, stroke_data.points[0].x);
					stroke_data.brush->set_state(STATE_Y, stroke_data.points[0].y);
					stroke_data.brush->set_state(STATE_PRESSURE, stroke_data.points[0].pressure);
					stroke_data.brush->stroke_to(&wrapper, stroke_data.points[0].x, stroke_data.points[0].y, stroke_data.points[0].pressure, 0.0f, 0.0f, 0.0f);
					for (auto& point : stroke_data.points) {
						stroke_data.brush->stroke_to(&wrapper, point.x, point.y, point.pressure, 0.0f, 0.0f, point.dtime);
					}
				}
			}
			// assign the restored surface to the layer
			if (layer->rendering_surface) {
				rendering::SurfaceResource::LockWrite<rendering::SurfaceSW> lock(layer->rendering_surface);
				if (lock) {
					lock->get_surface() = restored_surface;
					layer->changed();
				}
			}
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

			Surface restored_surface;
			int redraw_start = 0;
			if (checkpoint_idx != -1) {
				restored_surface = *strokes_history[checkpoint_idx].checkpoint_surface;
				redraw_start = checkpoint_idx;
			} else {
				auto it = original_layer_surface.find(layer);
				if (it != original_layer_surface.end()) {
					restored_surface = it->second;
				}
			}

			// replay strokes that belong to this layer since the checkpoint
			for (int i = redraw_start; i < strokes_history.size(); ++i) {
				StrokeData& stroke_data = strokes_history[i];
				if (stroke_data.layer != layer) continue;

				brushlib::SurfaceWrapper wrapper(&restored_surface);
				for (int j = 0; j < STATE_COUNT; j++) {
					stroke_data.brush->set_state(j, 0);
				}
				stroke_data.brush->set_state(STATE_X, stroke_data.points[0].x);
				stroke_data.brush->set_state(STATE_Y, stroke_data.points[0].y);
				stroke_data.brush->set_state(STATE_PRESSURE, stroke_data.points[0].pressure);
				stroke_data.brush->stroke_to(&wrapper, stroke_data.points[0].x, stroke_data.points[0].y, stroke_data.points[0].pressure, 0.0f, 0.0f, 0.0f);
				for (auto &point : stroke_data.points) {
					stroke_data.brush->stroke_to(&wrapper, point.x, point.y, point.pressure, 0.0f, 0.0f, point.dtime);
				}
			}

			// assign the restored surface to the layer.
			if(layer->rendering_surface) {
				rendering::SurfaceResource::LockWrite<rendering::SurfaceSW> lock(layer->rendering_surface);
				if (lock) {
					lock->get_surface() = restored_surface;
					layer->changed();
				}
			}

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
	if (!stroke.is_prepared()) stroke.prepare();
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

	applied = true;
	if (get_canvas_interface()) {
		get_canvas_interface()->signal_layer_param_changed()(stroke.get_layer(), "rendering_surface");
		get_canvas_interface()->get_selection_manager()->clear_selected_layers();
		get_canvas_interface()->get_selection_manager()->set_selected_layer(stroke.get_layer());
	}
}

void
Action::LayerBrush::undo()
{
	if (applied) {
		stroke.undo();
		applied = false;
		if (get_canvas_interface()) {
			get_canvas_interface()->signal_layer_param_changed()(stroke.get_layer(), "rendering_surface");
			// Reselect the layer
			get_canvas_interface()->get_selection_manager()->clear_selected_layers();
			get_canvas_interface()->get_selection_manager()->set_selected_layer(stroke.get_layer());
		}
	}
}