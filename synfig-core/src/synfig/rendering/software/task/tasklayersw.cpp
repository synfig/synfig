/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/tasklayersw.cpp
**	\brief TaskLayerSW
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "tasklayersw.h"

#include "../surfacesw.h"
#include <synfig/guid.h>
#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/debug/debugsurface.h>

#include <synfig/layers/layer_rendering_task.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
TaskLayerSW::run(RunParams & /* params */) const
{
	assert(layer);

	synfig::Surface &target =
		SurfaceSW::Handle::cast_dynamic( target_surface )->get_surface();

	Vector upp = get_units_per_pixel();
	Vector lt = get_source_rect_lt();
	Vector rb = get_source_rect_rb();
	lt[0] -= get_target_rect().minx*upp[0];
	lt[1] -= get_target_rect().miny*upp[1];
	rb[0] += (target.get_w() - get_target_rect().maxx)*upp[0];
	rb[1] += (target.get_h() - get_target_rect().maxy)*upp[1];

	RendDesc desc;
	desc.set_tl(lt);
	desc.set_br(rb);
	desc.set_wh(target.get_w(), target.get_h());
	desc.set_antialias(1);

	etl::handle<Layer_RenderingTask> sub_layer(new Layer_RenderingTask());
	sub_layer->tasks = sub_tasks;

	//for(List::const_iterator i = sub_tasks.begin(); i != sub_tasks.end(); ++i)
	//	if ((*i) && (*i)->valid_target())
	//		debug::DebugSurface::save_to_file(SurfaceSW::Handle::cast_dynamic( (*i)->target_surface )->get_surface(), "TaskLayerSW__run__sub");

	CanvasBase fake_canvas_base;
	fake_canvas_base.push_back(layer);
	fake_canvas_base.push_back(sub_layer);
	fake_canvas_base.push_back(Layer::Handle());

	Context context(fake_canvas_base.begin(), ContextParams());
	bool result = context.accelerated_render(&target, 4, desc, NULL);

	//debug::DebugSurface::save_to_file(target, "TaskLayerSW__run__target");

	return result;
}

/* === E N T R Y P O I N T ================================================= */
