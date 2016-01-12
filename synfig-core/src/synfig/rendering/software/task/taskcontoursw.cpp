/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskcontoursw.cpp
**	\brief TaskContourSW
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

#include "taskcontoursw.h"

#include "../surfacesw.h"
#include "../../optimizer.h"

#include "../function/contour.h"
#include <synfig/debug/debugsurface.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
TaskContourSW::split(const RectInt &sub_target_rect)
{
	trunc_target_rect(sub_target_rect);
}

bool
TaskContourSW::run(RunParams & /* params */) const
{
	synfig::Surface &a =
		SurfaceSW::Handle::cast_dynamic( target_surface )->get_surface();

	if (valid_target())
	{
		Matrix bounds_transfromation;
		bounds_transfromation.m00 = get_pixels_per_unit()[0];
		bounds_transfromation.m11 = get_pixels_per_unit()[1];
		bounds_transfromation.m20 = -get_source_rect_lt()[0]*bounds_transfromation.m00 + get_target_rect().minx;
		bounds_transfromation.m21 = -get_source_rect_lt()[1]*bounds_transfromation.m11 + get_target_rect().miny;

		Matrix matrix = transformation * bounds_transfromation;

		Polyspan polyspan;
		polyspan.init(get_target_rect());
		software::Contour::build_polyspan(contour->get_chunks(), matrix, polyspan);
		polyspan.sort_marks();

		software::Contour::render_polyspan(
			a,
			polyspan,
			contour->invert,
			contour->antialias,
			contour->winding_style,
			contour->color,
			blend ? amount : 1.0,
			blend ? blend_method : Color::BLEND_COMPOSITE );

		//debug::DebugSurface::save_to_file(a, "TaskContourSW__run");
	}

	return true;
}

/* === E N T R Y P O I N T ================================================= */
