/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskblursw.cpp
**	\brief TaskBlurSW
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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


Task::Token TaskBlurSW::token<TaskBlurSW, TaskBlur, TaskBlur>("BlurSW");


bool
TaskBlurSW::run(RunParams & /* params */) const
{
	if (!is_valid() || !sub_task() || !sub_task()->is_valid())
		return true;

	SurfaceResource::LockWrite<SurfaceSW> la(target_surface);
	SurfaceResource::LockRead<SurfaceSW> lb(sub_task()->target_surface);
	if (!la || !lb)
		return false;

	Vector ppu = get_pixels_per_unit();
	Vector s = blur.size.multiply_coords(ppu);

	Vector offsetf = (source_rect.get_min() - sub_task()->source_rect.get_min()).multiply_coords(ppu);
	VectorInt offset((int)round(offsetf[0]), (int)round(offsetf[1]));
	offset += sub_task()->target_rect.get_min();

	software::Blur::blur(
		software::Blur::Params(
			la->get_surface(), target_rect,
			lb->get_surface(), offset,
			blur.type, s,
			blend, blend_method, amount ));

	return false;
}

/* === E N T R Y P O I N T ================================================= */
