/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskpixelcolormatrixsw.cpp
**	\brief TaskPixelColorMatrixSW
**
**	$Id$
**
**	\legal
**	......... ... 2016-2018 Ivan Mahonin
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


Task::Token TaskPixelColorMatrixSW::token<TaskPixelColorMatrixSW, TaskPixelColorMatrix, TaskPixelColorMatrix>("PixelColorMatrixSW");


bool
TaskPixelColorMatrixSW::run(RunParams & /* params */) const
{
	if (!is_valid())
		return true;

	RectInt rd = target_rect;
	ColorMatrix::BatchProcessor processor(matrix);
	std::vector<RectInt> constant_rects(1, rd);

	SurfaceResource::LockWrite<SurfaceSW> ldst(target_surface);
	if (!ldst) return false;
	synfig::Surface &dst = ldst->get_surface();

	if (!processor.is_constant() && sub_task() && sub_task()->is_valid())
	{
		VectorInt offset = get_offset();
		RectInt rs = sub_task()->target_rect + rd.get_min() + offset;
		etl::set_intersect(rs, rs, rd);
		if (rs.is_valid())
		{
			SurfaceResource::LockRead<SurfaceSW> lsrc(target_surface);
			if (!lsrc) return false;
			const synfig::Surface &src = lsrc->get_surface();

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

	for(std::vector<RectInt>::const_iterator i = constant_rects.begin(); i != constant_rects.end(); ++i)
		dst.fill(processor.get_constant_value(), i->minx, i->miny, i->get_width(), i->get_height());

	return true;
}

/* === E N T R Y P O I N T ================================================= */
