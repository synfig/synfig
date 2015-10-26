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
TaskBlendSW::run(RunParams & /* params */) const
{
	const synfig::Surface &a =
		SurfaceSW::Handle::cast_dynamic( sub_task_a()->target_surface )->get_surface();
	const synfig::Surface &b =
		SurfaceSW::Handle::cast_dynamic( sub_task_b()->target_surface )->get_surface();
	synfig::Surface &c =
		SurfaceSW::Handle::cast_dynamic( target_surface )->get_surface();

	//debug::DebugSurface::save_to_file(a, "TaskBlendSW__run__a");
	//debug::DebugSurface::save_to_file(b, "TaskBlendSW__run__b");

	RectInt ra = sub_task_a()->target_rect;
	if (ra.valid() && &a != &c)
	{
		synfig::Surface::pen p = c.get_pen(
			ra.minx + offset_a[0],
			ra.miny + offset_a[1] );
		const_cast<synfig::Surface*>(&a)->blit_to(
			p, ra.minx, ra.miny, ra.maxx - ra.minx, ra.maxy - ra.miny );
	}

	RectInt rb = sub_task_b()->target_rect;
	if (rb.valid())
	{
		synfig::Surface::alpha_pen ap(c.get_pen(
			rb.minx + offset_b[0],
			rb.miny + offset_b[1] ));
		ap.set_blend_method(blend_method);
		ap.set_alpha(amount);
		const_cast<synfig::Surface*>(&b)->blit_to(
			ap, rb.minx, rb.miny, rb.maxx - rb.minx, rb.maxy - rb.miny );
	}

	//debug::DebugSurface::save_to_file(c, "TaskBlendSW__run__c");

	return true;
}

/* === E N T R Y P O I N T ================================================= */
