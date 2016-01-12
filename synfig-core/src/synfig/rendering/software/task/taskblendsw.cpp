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
#include <synfig/general.h>

#include "taskblendsw.h"
#include "../surfacesw.h"
#include "../../optimizer.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
TaskBlendSW::split(const RectInt &sub_target_rect)
{
	RectInt prev_target_rect = get_target_rect();
	trunc_target_rect(sub_target_rect);
	offset_a += prev_target_rect.get_min() - get_target_offset();
	offset_b += prev_target_rect.get_min() - get_target_offset();
	if (valid_target())
	{
		if (sub_task_a() && sub_task_a()->valid_target())
		{
			// TODO: Buggg! Here we should to call "split" if possible
			// TODO: solve problem with offset_a and offset_b
			sub_task_a() = sub_task_a()->clone();
			sub_task_a()->trunc_target_rect(
				get_target_rect()
				- get_target_offset()
				- offset_a );
		}
		if (sub_task_b() && sub_task_b()->valid_target())
		{
			sub_task_b() = sub_task_a()->clone();
			sub_task_b()->trunc_target_rect(
				get_target_rect()
				- get_target_offset()
				- offset_b );
		}
	}
}

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

	RectInt r = get_target_rect();
	if (r.valid())
	{
		RectInt ra = sub_task_a()->get_target_rect() + r.get_min() + offset_a;
		if (ra.valid())
		{
			etl::set_intersect(ra, ra, r);
			if (ra.valid() && &a != &c)
			{
				synfig::Surface::pen p = c.get_pen(ra.minx, ra.maxx);
				const_cast<synfig::Surface*>(&a)->blit_to(
					p,
					ra.minx - r.minx - offset_a[0],
					ra.miny - r.miny - offset_a[1],
					ra.maxx - ra.minx,
					ra.maxy - ra.miny );
			}
		}

		RectInt fill[] = { ra, RectInt::zero(), RectInt::zero(), RectInt::zero() };
		RectInt rb = sub_task_b()->get_target_rect() + r.get_min() + offset_b;
		if (rb.valid())
		{
			etl::set_intersect(rb, rb, r);
			if (rb.valid())
			{
				synfig::Surface::alpha_pen ap(c.get_pen(rb.minx, rb.miny));
				ap.set_blend_method(blend_method);
				ap.set_alpha(amount);
				const_cast<synfig::Surface*>(&b)->blit_to(
					ap,
					rb.minx - r.minx - offset_b[0],
					rb.miny - r.miny - offset_b[1],
					rb.maxx - rb.minx,
					rb.maxy - rb.miny );

				if (ra.valid())
				{
					// mark unfilled regions
					fill[0] = fill[1] = fill[2] = fill[3] = ra;
					fill[0].maxx = fill[2].minx = fill[3].minx = std::max(ra.minx, std::min(ra.maxx, rb.minx));
					fill[1].minx = fill[2].maxx = fill[3].maxx = std::max(ra.minx, std::min(ra.maxx, rb.maxx));
					fill[2].maxy = std::max(ra.miny, std::min(ra.maxy, rb.miny));
					fill[3].miny = std::max(ra.miny, std::min(ra.maxy, rb.maxy));
				}
			}
		}

		if (Color::is_straight(blend_method))
		{
			for(int i = 0; i < 4; ++i)
			{
				if (fill[i].valid())
				{
					synfig::Surface::alpha_pen ap(
						c.get_pen(fill[i].minx, fill[i].miny) );
					ap.set_blend_method(blend_method);
					ap.set_alpha(amount);
					c.fill( Color(0, 0, 0, 0), ap,
							fill[i].maxx - fill[i].minx,
							fill[i].maxy - fill[i].miny );
				}
			}
		}
	}

	//debug::DebugSurface::save_to_file(c, "TaskBlendSW__run__c");

	return true;
}

/* === E N T R Y P O I N T ================================================= */
