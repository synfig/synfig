/* === S Y N F I G ========================================================= */
/*!	\file layer_rendering_task.cpp
**	\brief Layer_RenderingTask implementation
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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
#include <synfig/localization.h>

#include "layer_rendering_task.h"

#include <synfig/context.h>
#include <synfig/rendering/software/surfacesw.h>

#include <synfig/debug/debugsurface.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Layer_RenderingTask::Layer_RenderingTask(): renderer(NULL) { }

Rect
Layer_RenderingTask::get_bounding_rect() const
{
	return task ? task->get_bounds() : Rect::zero();
}

bool
Layer_RenderingTask::accelerated_render(Context /* context */, Surface *surface, int /* quality */, const RendDesc &renddesc, ProgressCallback * /* cb */) const
{
	if (!renderer || !task) return false;

	assert(surface);

	surface->set_wh(renddesc.get_w(), renddesc.get_h());
	surface->clear();

	rendering::SurfaceSW::Handle target_surface(new rendering::SurfaceSW());
	target_surface->set_surface(*surface);
	assert(target_surface->is_created());

	task->target_surface = target_surface;
	task->target_surface->create();
	task->init_target_rect(RectInt(VectorInt::zero(), target_surface->get_size()), renddesc.get_tl(), renddesc.get_br());

	rendering::Task::List list;
	list.push_back(task);
	renderer->run(list);

	//debug::DebugSurface::save_to_file(*surface, "Layer_RenderingTask");

	return true;
}
