/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskblendsw.cpp
**	\brief TaskBlendSW
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/debug/debugsurface.h>

#include "taskblendsw.h"
#include "../surfacesw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
TaskBlendSW::run(RunParams &params) const
{
	const synfig::Surface &a =
		SurfaceSW::Handle::cast_dynamic( sub_task_a()->target_surface )->get_surface();
	const synfig::Surface &b =
		SurfaceSW::Handle::cast_dynamic( sub_task_b()->target_surface )->get_surface();
	synfig::Surface &c =
		SurfaceSW::Handle::cast_dynamic( target_surface )->get_surface();

	//debug::DebugSurface::save_to_file(a, "TaskBlendSW__run__a");
	//debug::DebugSurface::save_to_file(b, "TaskBlendSW__run__b");

	const etl::rect<int> &ra = sub_task_a()->target_surface->used_rect;
	const etl::rect<int> &rb = sub_task_b()->target_surface->used_rect;
	etl::rect<int> used_rect_a = ra;
	etl::rect<int> used_rect_b = params.used_rect;

	if (!Color::is_straight(blend_method) && Color::is_onto(blend_method))
	{
		if (ra.valid() && rb.valid())
			set_intersect(params.used_rect, ra, rb);
		else
			params.used_rect = etl::rect<int>(0, 0, 0, 0);
		used_rect_a = params.used_rect;
		used_rect_b = params.used_rect;
	}
	else
	if (!Color::is_straight(blend_method))
	{
		if (ra.valid() && rb.valid())
			set_union(params.used_rect, ra, rb);
		else
			params.used_rect = ra.valid() ? ra : rb;
		used_rect_a = ra;
		used_rect_b = rb;
	}
	else
	if (Color::is_onto(blend_method))
	{
		params.used_rect = ra;
		used_rect_a = ra;
		used_rect_b = ra;
	}

	if (params.used_rect.valid())
	{
		int x = used_rect_a.minx;
		int y = used_rect_a.miny;
		int w = used_rect_a.maxx - x;
		int h = used_rect_a.maxy - y;

		if (&a != &c)
		{
			synfig::Surface::pen p = c.get_pen(x, y);
			const_cast<synfig::Surface*>(&a)->blit_to(p, x, y, w, h);
		}

		x = used_rect_b.minx;
		y = used_rect_b.miny;
		w = used_rect_b.maxx - x;
		h = used_rect_b.maxy - y;

		synfig::Surface::alpha_pen ap(c.get_pen(x, y));
		ap.set_blend_method(blend_method);
		ap.set_alpha(amount);
		const_cast<synfig::Surface*>(&b)->blit_to(ap, x, y, w, h);
	}

	//debug::DebugSurface::save_to_file(c, "TaskBlendSW__run__c");

	return true;
}

/* === E N T R Y P O I N T ================================================= */
