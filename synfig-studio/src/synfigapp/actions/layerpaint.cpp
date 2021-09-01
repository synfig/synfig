/* === S Y N F I G ========================================================= */
/*!	\file layerpaint.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include "layerpaint.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>
#include <synfigapp/instance.h>

#include <synfig/layers/layer_pastecanvas.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/rendering/software/surfacesw.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerPaint);
ACTION_SET_NAME(Action::LayerPaint,"LayerPaint");
ACTION_SET_LOCAL_NAME(Action::LayerPaint,N_("Paint"));
ACTION_SET_TASK(Action::LayerPaint,"paint");
ACTION_SET_CATEGORY(Action::LayerPaint,Action::CATEGORY_NONE);
ACTION_SET_PRIORITY(Action::LayerPaint,0);
ACTION_SET_VERSION(Action::LayerPaint,"0.0");

/* === G L O B A L S ======================================================= */

Action::LayerPaint::PaintStroke* Action::LayerPaint::PaintStroke::first = NULL;
Action::LayerPaint::PaintStroke* Action::LayerPaint::PaintStroke::last = NULL;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerPaint::PaintStroke::PaintStroke():
	prev(NULL),
	next(NULL),
	prevSameLayer(NULL),
	nextSameLayer(NULL),
	prepared(false),
	applied(false)
{
}

Action::LayerPaint::PaintStroke::~PaintStroke()
{
	if (prepared)
	{
		if (nextSameLayer != NULL)
		{
			if (prevSameLayer == NULL)
				paint_self(nextSameLayer->surface);
			else
				nextSameLayer->points.insert(nextSameLayer->points.begin(), points.begin(), points.end());
			nextSameLayer->prevSameLayer = prevSameLayer;
		}
		if (prevSameLayer != NULL) prevSameLayer->nextSameLayer = nextSameLayer;
		if (prev == NULL) first = next; else prev->next = next;
		if (next == NULL) last = prev; else next->prev = prev;
	}
}

void
Action::LayerPaint::PaintStroke::paint_prev(synfig::Surface &surface)
{
	if (prevSameLayer == NULL) {
		surface = this->surface;
		return;
	}
	prevSameLayer->paint_self(surface);
}

void
Action::LayerPaint::PaintStroke::paint_self(synfig::Surface &surface)
{
	paint_prev(surface);
	brushlib::SurfaceWrapper wrapper(&surface);
	if (!points.empty()) reset(points.front());
	for(std::vector<PaintPoint>::const_iterator i = points.begin(); i != points.end(); i++)
	{
		brush_.stroke_to(&wrapper, i->x, i->y, i->pressure, 0.f, 0.f, i->dtime);
		wrapper.offset_x = 0;
		wrapper.offset_y = 0;
	}
}

void Action::LayerPaint::PaintStroke::reset(const PaintPoint &point)
{
    for (int i=0; i<STATE_COUNT; i++)
    	brush_.set_state(i, 0);
    brush_.set_state(STATE_X, point.x);
    brush_.set_state(STATE_Y, point.y);
    brush_.set_state(STATE_PRESSURE, point.pressure);
    brush_.set_state(STATE_ACTUAL_X, brush_.get_state(STATE_X));
    brush_.set_state(STATE_ACTUAL_Y, brush_.get_state(STATE_Y));
    brush_.set_state(STATE_STROKE, 1.0); // start in a state as if the stroke was long finished
}

void
Action::LayerPaint::PaintStroke::add_point_and_apply(const PaintPoint &point)
{
	assert(prepared);
	assert(applied || points.empty());
	assert(prevSameLayer == NULL || prevSameLayer->applied);
	assert(nextSameLayer == NULL);

	if (points.empty()) reset(point);
	points.push_back(point);
	applied = true;

	Surface *surface = new Surface();
	{
		rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(layer->rendering_surface);
		if (lock) surface->copy( lock->get_surface() );
	}

	brushlib::SurfaceWrapper wrapper(surface);
	int w = wrapper.surface->get_w();
	int h = wrapper.surface->get_h();
	{
		std::lock_guard<std::mutex> lock(layer->mutex);
		brush_.stroke_to(&wrapper, point.x, point.y, point.pressure, 0.f, 0.f, point.dtime);

		// fix state in case of surface resized
		float x = brush_.get_state(STATE_X) + wrapper.offset_x;
		float y = brush_.get_state(STATE_Y) + wrapper.offset_y;
		brush_.set_state(STATE_X, x);
		brush_.set_state(STATE_Y, y);
		brush_.set_state(STATE_ACTUAL_X, x);
		brush_.set_state(STATE_ACTUAL_Y, y);

		layer->rendering_surface = new rendering::SurfaceResource(
				new rendering::SurfaceSW(*surface, true) );
	}

	if (wrapper.extra_left > 0 || wrapper.extra_top > 0) {
		new_tl -= Point(
			(Real)wrapper.extra_left/(Real)w*(new_br[0] - new_tl[0]),
			(Real)wrapper.extra_top/(Real)h*(new_br[1] - new_tl[1]) );
		layer->set_param("tl", ValueBase(new_tl));
	}
	if (wrapper.extra_right > 0 || wrapper.extra_bottom > 0) {
		new_br += Point(
			(Real)wrapper.extra_right/(Real)w*(new_br[0] - new_tl[0]),
			(Real)wrapper.extra_bottom/(Real)h*(new_br[1] - new_tl[1]) );
		layer->set_param("br", ValueBase(new_br));
	}
	layer->changed();
}

void
Action::LayerPaint::PaintStroke::prepare()
{
	assert(layer);
	assert(!prepared);

	prev = last; last = this;
	if (prev == NULL) first = this; else prev->next = this;

	for(PaintStroke *p = prev; p != NULL; p = p->prev)
		if (p->layer == layer)
		{
			assert(p->nextSameLayer == NULL);
			prevSameLayer = p;
			p->nextSameLayer = this;
			break;
		}

	if (prevSameLayer == NULL) {
		rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(layer->rendering_surface);
		if (lock) surface = lock->get_surface();
	}
	new_tl = tl = layer->get_param("tl").get(Point());
	new_br = br = layer->get_param("br").get(Point());

	prepared = true;
}


void
Action::LayerPaint::PaintStroke::undo()
{
	assert(prepared);
	if (!applied) return;
	{
		std::lock_guard<std::mutex> lock(layer->mutex);
		Surface *surface = new Surface();
		paint_prev(*surface);
		layer->rendering_surface = new rendering::SurfaceResource(
				new rendering::SurfaceSW(*surface, true) );
	}
	applied = false;
	layer->set_param("tl", ValueBase(tl));
	layer->set_param("br", ValueBase(br));
	layer->changed();
}

void
Action::LayerPaint::PaintStroke::apply()
{
	assert(prepared);
	if (applied) return;
	{
		std::lock_guard<std::mutex> lock(layer->mutex);
		Surface *surface = new Surface();
		paint_self(*surface);
		layer->rendering_surface = new rendering::SurfaceResource(
				new rendering::SurfaceSW(*surface, true) );
	}
	applied = true;
	layer->set_param("tl", ValueBase(new_tl));
	layer->set_param("br", ValueBase(new_br));
	layer->changed();
}



Action::LayerPaint::LayerPaint():
	applied(false)
{ }

Action::ParamVocab
Action::LayerPaint::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	return ret;
}

bool
Action::LayerPaint::is_candidate(const ParamList & /* x */)
{
	return false;
}

bool
Action::LayerPaint::set_param(const synfig::String& name, const Action::Param &param)
{
	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerPaint::is_ready()const
{
	if(!stroke.is_prepared())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerPaint::perform()
{
	assert(!applied);
	stroke.apply();
	if (!applied) stroke.get_layer()->add_surface_modification_id(id);
	applied = !applied;
}

void
Action::LayerPaint::undo()
{
	assert(applied);
	stroke.undo();
	if (applied) stroke.get_layer()->add_surface_modification_id(id);
	applied = !applied;
}
