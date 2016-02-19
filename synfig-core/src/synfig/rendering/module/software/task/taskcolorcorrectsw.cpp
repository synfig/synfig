/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/module/software/task/taskcolorcorrectsw.cpp
**	\brief TaskColorCorrectSW
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

#include "taskcolorcorrectsw.h"

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
TaskColorCorrectSW::split(const RectInt &sub_target_rect)
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
TaskColorCorrectSW::correct_pixel(Color &dst, const Color &src, const Angle &hue_adjust, ColorReal shift, ColorReal amplifier, const Gamma &gamma) const
{
	static const double precision = 1e-8;

	dst = src;

	if (fabs(gamma.get_gamma_r() - 1.0) > precision)
	{
		if (dst.get_r() < 0)
			dst.set_r(-gamma.r_F32_to_F32(-dst.get_r()));
		else
			dst.set_r(gamma.r_F32_to_F32(dst.get_r()));
	}

	if (fabs(gamma.get_gamma_g() - 1.0) > precision)
	{
		if (dst.get_g() < 0)
			dst.set_g(-gamma.g_F32_to_F32(-dst.get_g()));
		else
			dst.set_g(gamma.g_F32_to_F32(dst.get_g()));
	}

	if (fabs(gamma.get_gamma_b() - 1.0) > precision)
	{
		if (dst.get_b() < 0)
			dst.set_b(-gamma.b_F32_to_F32(-dst.get_b()));
		else
			dst.set_b(gamma.b_F32_to_F32(dst.get_b()));
	}

	assert(!isnan(dst.get_r()));
	assert(!isnan(dst.get_g()));
	assert(!isnan(dst.get_b()));

	if (fabs(amplifier - 1.0) > precision)
	{
		dst.set_r(dst.get_r()*amplifier);
		dst.set_g(dst.get_g()*amplifier);
		dst.set_b(dst.get_b()*amplifier);
	}

	if (fabs(shift) > precision)
	{
		// Adjust R Channel Brightness
		if (dst.get_r() > -shift)
			dst.set_r(dst.get_r() + shift);
		else
		if(dst.get_r() < shift)
			dst.set_r(dst.get_r() - shift);
		else
			dst.set_r(0);

		// Adjust G Channel Brightness
		if (dst.get_g() > -shift)
			dst.set_g(dst.get_g() + shift);
		else
		if(dst.get_g() < shift)
			dst.set_g(dst.get_g() - shift);
		else
			dst.set_g(0);

		// Adjust B Channel Brightness
		if (dst.get_b() > -shift)
			dst.set_b(dst.get_b() + shift);
		else
		if(dst.get_b() < shift)
			dst.set_b(dst.get_b() - shift);
		else
			dst.set_b(0);
	}

	// Return the color, adjusting the hue if necessary
	if (!!hue_adjust)
		dst.rotate_uv(hue_adjust);
}

bool
TaskColorCorrectSW::run(RunParams & /* params */) const
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
				ColorReal amplifier = (ColorReal)(contrast*exp(exposure));
				ColorReal shift = (ColorReal)((brightness - 0.5)*contrast + 0.5);
				Gamma g(fabs(gamma) < 1e-8 ? 1.0 : 1.0/gamma);

				synfig::Surface::pen pc = c.get_pen(ra.minx, ra.maxx);
				synfig::Surface::pen pa = c.get_pen(ra.minx, ra.maxx);
				for(int y = ra.miny; y < ra.maxy; ++y)
				{
					const Color *ca = &a[y - r.miny + offset[1]][ra.minx - r.minx + offset[0]];
					Color *cc = &c[y][ra.minx];
					for(int x = ra.minx; x < ra.maxx; ++x, ++ca, ++cc)
						correct_pixel(*cc, *ca, hue_adjust, shift, amplifier, g);
				}
			}
		}
	}

	//debug::DebugSurface::save_to_file(c, "TaskClampSW__run__c");

	return true;
}

/* === E N T R Y P O I N T ================================================= */
