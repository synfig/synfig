/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskpixelcolormatrixsw.cpp
**	\brief TaskPixelColorMatrixSW
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

#include "taskpixelcolormatrixsw.h"
#include "../surfacesw.h"
#include "../../optimizer.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/*
void
TaskPixelColorMatrixSW::split(const RectInt &sub_target_rect)
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
*/

bool
TaskPixelColorMatrixSW::run(RunParams & /* params */) const
{
	synfig::Surface &dst =
		rendering::SurfaceSW::Handle::cast_dynamic( target_surface )->get_surface();

	RectInt rd = get_target_rect();
	if (rd.valid())
	{
		ColorMatrix::BatchProcessor processor(matrix);
		std::vector<RectInt> constant_rects(1, rd);

		if (!processor.is_constant() && sub_task() && sub_task()->valid_target())
		{
			VectorInt offset = get_offset();
			RectInt rs = sub_task()->get_target_rect() + rd.get_min() + offset;
			if (rs.valid())
			{
				etl::set_intersect(rs, rs, rd);
				if (rs.valid())
				{
					const synfig::Surface &src =
						rendering::SurfaceSW::Handle::cast_dynamic( sub_task()->target_surface )->get_surface();
					rs.list_subtract(constant_rects);
					processor.process(
						&dst[rs.miny][rs.minx],
						dst.get_pitch()/sizeof(Color),
						&src[rs.miny - rd.miny - offset[1]][rs.minx - rd.minx - offset[0]],
						src.get_pitch()/sizeof(Color),
						rs.get_width(),
						rs.get_height() );
				}
			}
		}

		for(std::vector<RectInt>::const_iterator i = constant_rects.begin(); i != constant_rects.end(); ++i)
			dst.fill(processor.get_constant_value(), i->minx, i->miny, i->get_width(), i->get_height());
	}

	return true;
}

/* === E N T R Y P O I N T ================================================= */
