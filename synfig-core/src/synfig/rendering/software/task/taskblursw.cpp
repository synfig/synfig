/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskblursw.cpp
**	\brief TaskBlurSW
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

#include "taskblursw.h"

#include "../surfacesw.h"
#include "../function/blur.h"

#include <synfig/general.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
TaskBlurSW::run(RunParams & /* params */) const
{
	if (!valid_target() || !sub_task()->valid_target())
		return true;

	synfig::Surface &a =
		SurfaceSW::Handle::cast_dynamic( target_surface )->get_surface();
	const synfig::Surface &b =
		SurfaceSW::Handle::cast_dynamic( sub_task()->target_surface )->get_surface();

	Vector s = blur.size;
	Vector pixels_per_unit = get_pixels_per_unit();
	s[0] *= pixels_per_unit[0];
	s[1] *= pixels_per_unit[1];

	Vector offsetf = get_source_rect_lt() - sub_task()->get_source_rect_lt();
	Vector sub_pixels_per_unit = sub_task()->get_pixels_per_unit();
	offsetf[0] *= sub_pixels_per_unit[0];
	offsetf[1] *= sub_pixels_per_unit[1];
	VectorInt offset((int)round(offsetf[0]), (int)round(offsetf[1]));
	offset += sub_task()->get_target_rect().get_min();

	software::Blur::blur(
		software::Blur::Params(
			a, get_target_rect(),
			b, offset,
			blur.type, s,
			blend, blend_method, amount ));

	return false;
}

/* === E N T R Y P O I N T ================================================= */
