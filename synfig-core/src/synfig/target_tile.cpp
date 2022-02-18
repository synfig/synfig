/* === S Y N F I G ========================================================= */
/*!	\file target_tile.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#include <vector>
#include <algorithm>

#include <ETL/clock>

#include "target_tile.h"

#include "general.h"
#include "localization.h"

#include "canvas.h"
#include "context.h"
#include "render.h"
#include "string.h"
#include "surface.h"

#include "debug/measure.h"

#include "rendering/renderer.h"
#include "rendering/surface.h"
#include "rendering/software/surfacesw.h"
#include "rendering/common/task/tasktransformation.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

const unsigned int	DEF_TILE_WIDTH = TILE_SIZE / 2;
const unsigned int	DEF_TILE_HEIGHT = TILE_SIZE / 2;

#ifdef _DEBUG
//#define DEBUG_MEASURE
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Target_Tile::Target_Tile():
	threads_(2),
	tile_w_(DEF_TILE_WIDTH),
	tile_h_(DEF_TILE_HEIGHT),
	curr_tile_(0),
	clipping_(true)
{
	curr_frame_=0;
	if (const char *s = getenv("SYNFIG_TARGET_DEFAULT_ENGINE"))
		set_engine(s);
}

int
Target_Tile::next_frame(Time& time)
{
	return Target::next_frame(time);
}

int
Target_Tile::next_tile(RectInt& rect)
{
	// Width of the image(in tiles)
	int tw(rend_desc().get_w()/tile_w_);
	int th(rend_desc().get_h()/tile_h_);

	// Add the last tiles (which will be clipped)
	if(rend_desc().get_w()%tile_w_!=0)tw++;
	if(rend_desc().get_h()%tile_h_!=0)th++;

	rect.minx = (curr_tile_%tw)*tile_w_;
	rect.miny = (curr_tile_/tw)*tile_h_;
	rect.maxx = rect.minx + tile_w_;
	rect.maxy = rect.miny + tile_h_;

	curr_tile_++;
	return (tw*th)-curr_tile_+1;
}

bool
synfig::Target_Tile::call_renderer(
	const etl::handle<rendering::SurfaceResource> &surface,
	Canvas &canvas,
	const ContextParams &context_params,
	const RendDesc &renddesc )
{
	#ifdef DEBUG_MEASURE
	debug::Measure t("Target_Tile::call_renderer");
	#endif

	surface->create(renddesc.get_w(), renddesc.get_h());
	rendering::Task::Handle task;
	{
		#ifdef DEBUG_MEASURE
		debug::Measure t("build rendering task");
		#endif
		task = canvas.build_rendering_task(context_params);
	}

	if (task)
	{
		rendering::Renderer::Handle renderer = rendering::Renderer::get_renderer(get_engine());
		if (!renderer)
			throw "Renderer '" + get_engine() + "' not found";

		Vector p0 = renddesc.get_tl();
		Vector p1 = renddesc.get_br();
		if (p0[0] > p1[0] || p0[1] > p1[1]) {
			Matrix m;
			if (p0[0] > p1[0]) { m.m00 = -1.0; m.m20 = p0[0] + p1[0]; std::swap(p0[0], p1[0]); }
			if (p0[1] > p1[1]) { m.m11 = -1.0; m.m21 = p0[1] + p1[1]; std::swap(p0[1], p1[1]); }
			TaskTransformationAffine::Handle t = new TaskTransformationAffine();
			t->transformation->matrix = m;
			t->sub_task() = task;
			task = t;
		}

		task->target_surface = surface;
		task->target_rect = RectInt( VectorInt(), surface->get_size() );
		task->source_rect = Rect(p0, p1);

		rendering::Task::List list;
		list.push_back(task);

		{
			#ifdef DEBUG_MEASURE
			debug::Measure t("run renderer");
			#endif
			renderer->run(list);
		}
	}
	return true;
}

bool
synfig::Target_Tile::render_frame_(Canvas::Handle canvas, ContextParams context_params, ProgressCallback *cb)
{
	const RendDesc &rend_desc(desc);

	etl::clock tile_timer;
	tile_timer.reset();

	// Gather tiles
	std::vector<RectInt> tiles;
	RectInt rect;
	while(next_tile(rect)) {
		if (clipping_)
			if (rect.minx >= rend_desc.get_w() || rect.miny >= rend_desc.get_h())
				continue;
		tiles.push_back(rect);
	}

	// Render tiles
	for(std::vector<RectInt>::iterator i = tiles.begin(); i != tiles.end(); ++i)
	{
		// Progress callback
		int index = i - tiles.begin();
		int count = (int)tiles.size();
		SuperCallback super(cb, (count-index)*1000, (count-index+1)*1000, count*1000);
		if(!super.amount_complete(0,1000))
			return false;

		// Render tile
		tile_timer.reset();

		rect = *i;
		if (clipping_)
			rect_set_intersect(rect, rect, RectInt(0, 0, rend_desc.get_w(), rend_desc.get_h()));

		if (!rect.valid())
			continue;

		RendDesc tile_desc=rend_desc;
		tile_desc.set_subwindow(rect.minx, rect.miny, rect.maxx - rect.minx, rect.maxy - rect.miny);

		async_render_tile(canvas, context_params, rect, tile_desc, &super);
	}

	if (!wait_render_tiles(cb))
		return false;

	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}

bool
synfig::Target_Tile::async_render_tile(
	etl::handle<Canvas> canvas,
	ContextParams context_params,
	RectInt rect,
	RendDesc tile_desc,
	ProgressCallback *cb)
{
	SurfaceResource::Handle surface = new rendering::SurfaceResource();

	if (!call_renderer(surface, *canvas, context_params, tile_desc))
	{
		// For some reason, the accelerated renderer failed.
		if(cb)cb->error(_("Accelerated Renderer Failure"));
		return false;
	}

	SurfaceResource::LockWrite<SurfaceSW> lock(surface);

	if(!lock)
	{
		if(cb)cb->error(_("Bad surface"));
		return false;
	}

	synfig::Surface &s = lock->get_surface();
	int cnt = s.get_w() * s.get_h();

	switch(get_alpha_mode())
	{
		case TARGET_ALPHA_MODE_FILL:
			for(int i = 0; i < cnt; ++i)
				s[0][i] = Color::blend(s[0][i], desc.get_bg_color(), 1.0f);
			break;
		case TARGET_ALPHA_MODE_EXTRACT:
			for(int i = 0; i< cnt; ++i)
			{
				float a = s[0][i].get_a();
				s[0][i] = Color(a,a,a,a);
			}
			break;
		case TARGET_ALPHA_MODE_REDUCE:
			for(int i = 0; i < cnt; ++i)
				s[0][i].set_a(1.0f);
			break;
		default:
			break;
	}

	// Add the tile to the target
	if (!add_tile(s, rect.minx, rect.miny))
	{
		if(cb)cb->error(_("add_tile(): Unable to put surface on target"));
		return false;
	}

	signal_progress()();
	return true;
}

bool
synfig::Target_Tile::wait_render_tiles(ProgressCallback* /* cb */)
{
	return true;
}


bool
synfig::Target_Tile::render(ProgressCallback *cb)
{
	SuperCallback super_cb;
	int
		frames=0,
		total_frames,
		frame_start,
		frame_end;
	Time
		t=0;

	assert(canvas);
	curr_frame_=0;
	//init();
	if (!init()) {
		if (cb) cb->error(_("Target initialization failure"));
		return false;
	}

	frame_start=desc.get_frame_start();
	frame_end=desc.get_frame_end();

	ContextParams context_params(desc.get_render_excluded_contexts());

	// Calculate the number of frames
	total_frames=frame_end-frame_start+1;
	if(total_frames<=0)total_frames=1;

	try {

		if(total_frames>=1)
		{
			do
			{		
				// Grab the time
				frames=next_frame(t);

				curr_tile_=0;

				// If we have a callback, and it returns
				// false, go ahead and bail. (maybe a use cancel)
				if(cb && !cb->amount_complete(total_frames-frames,total_frames))
					return false;

				if(!start_frame(cb))
					return false;

				// Set the time that we wish to render
				canvas->set_time(t);
				canvas->load_resources(t);
				canvas->set_outline_grow(desc.get_outline_grow());
				if(!render_frame_(canvas, context_params, 0))
					return false;
				end_frame();
			}while(frames);
			//synfig::info("tilerenderer: i=%d, t=%s",i,t.get_string().c_str());
		}
		else
		{
			curr_tile_=0;

			if(!start_frame(cb))
				return false;

			// Set the time that we wish to render
			canvas->set_time(t);
			canvas->load_resources(t);
			canvas->set_outline_grow(desc.get_outline_grow());

			//synfig::info("2time_set_to %s",t.get_string().c_str());
			if(!render_frame_(canvas, context_params, cb))
				return false;
			end_frame();
		}

	}
	catch (const String& str)
	{
		if (cb) cb->error(_("Caught string: ")+str);
		return false;
	}
	catch (std::bad_alloc&)
	{
		if (cb) cb->error(_("Ran out of memory (Probably a bug)"));
		return false;
	}
	catch(...)
	{
		if (cb) cb->error(_("Caught unknown error, rethrowing..."));
		throw;
	}
	return true;
}
