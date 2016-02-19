/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/module/software/task/taskclampsw.cpp
**	\brief TaskClampSW
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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/debug/debugsurface.h>
#include <synfig/general.h>

#include "taskclampsw.h"

#include <synfig/rendering/optimizer.h>
#include <synfig/rendering/software/surfacesw.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
TaskClampSW::split(const RectInt &sub_target_rect)
{
	trunc_target_rect(sub_target_rect);
	if (valid_target() && sub_task() && sub_task()->valid_target())
	{
		sub_task() = sub_task()->clone();
		sub_task()->trunc_target_rect(
			get_target_rect()
			- get_target_offset()
			- get_offset() );
	}
}

void
TaskClampSW::clamp_pixel(Color &dst, const Color &src) const
{
	if (fabs(src.get_a()) < 1e-8)
		{ dst = Color::alpha(); return; }

	dst = src;

	if (invert_negative)
	{
		if (dst.get_a() < floor)
			dst = -dst;

		if(dst.get_r() < floor)
		{
			dst.set_g(dst.get_g() - dst.get_r());
			dst.set_b(dst.get_b() - dst.get_r());
			dst.set_r(floor);
		}

		if(dst.get_g() < floor)
		{
			dst.set_r(dst.get_r() - dst.get_g());
			dst.set_b(dst.get_b() - dst.get_g());
			dst.set_g(floor);
		}

		if(dst.get_b() < floor)
		{
			dst.set_g(dst.get_g() - dst.get_b());
			dst.set_r(dst.get_r() - dst.get_b());
			dst.set_b(floor);
		}
	}
	else
	if (clamp_floor)
	{
		if (dst.get_r() < floor) dst.set_r(floor);
		if (dst.get_g() < floor) dst.set_g(floor);
		if (dst.get_b() < floor) dst.set_b(floor);
		if (dst.get_a() < floor) dst.set_a(floor);
	}

	if (clamp_ceiling)
	{
		if (dst.get_r() > ceiling) dst.set_r(ceiling);
		if (dst.get_g() > ceiling) dst.set_g(ceiling);
		if (dst.get_b() > ceiling) dst.set_b(ceiling);
		if (dst.get_a() > ceiling) dst.set_a(ceiling);
	}
}

bool
TaskClampSW::run(RunParams & /* params */) const
{
	const synfig::Surface &a =
		SurfaceSW::Handle::cast_dynamic( sub_task()->target_surface )->get_surface();
	synfig::Surface &c =
		SurfaceSW::Handle::cast_dynamic( target_surface )->get_surface();

	//debug::DebugSurface::save_to_file(a, "TaskClampSW__run__a");

	RectInt r = get_target_rect();
	if (r.valid())
	{
		VectorInt offset = get_offset();
		RectInt ra = sub_task()->get_target_rect() + r.get_min() + get_offset();
		if (ra.valid())
		{
			etl::set_intersect(ra, ra, r);
			if (ra.valid())
			{
				synfig::Surface::pen pc = c.get_pen(ra.minx, ra.maxx);
				synfig::Surface::pen pa = c.get_pen(ra.minx, ra.maxx);
				for(int y = ra.miny; y < ra.maxy; ++y)
				{
					const Color *ca = &a[y - r.miny + offset[1]][ra.minx - r.minx + offset[0]];
					Color *cc = &c[y][ra.minx];
					for(int x = ra.minx; x < ra.maxx; ++x, ++ca, ++cc)
						clamp_pixel(*cc, *ca);
				}
			}
		}
	}

	//debug::DebugSurface::save_to_file(c, "TaskClampSW__run__c");

	return true;
}

/* === E N T R Y P O I N T ================================================= */
