/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskpixelprocessor.cpp
**	\brief TaskPixelProcessor
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

#include "taskpixelprocessor.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Rect
TaskPixelProcessor::calc_bounds() const
{
	return is_transparent() ? Rect::zero()
	     : is_affects_transparent() ? Rect::infinite()
	     : sub_task() ? sub_task()->get_bounds()
	     : Rect::zero();
}

VectorInt
TaskPixelProcessor::get_offset() const
{
	if (!sub_task()) return VectorInt::zero();
	Vector offset = (sub_task()->get_source_rect_lt() - get_source_rect_lt()).multiply_coords(get_pixels_per_unit());
	return VectorInt((int)round(offset[0]), (int)round(offset[1])) - sub_task()->get_target_offset();
}

/* === E N T R Y P O I N T ================================================= */
