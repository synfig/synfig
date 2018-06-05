/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskblend.cpp
**	\brief TaskBlend
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

#include "taskblend.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Task::Token TaskBlend::token(
	DescAbstract<TaskBlend>("Blend") );


Rect
TaskBlend::calc_bounds() const
{
	Rect ra = sub_task_a() ? sub_task_a()->get_bounds() : Rect::zero();
	Rect rb = sub_task_b() ? sub_task_b()->get_bounds() : Rect::zero();
	Rect bounds = Rect::zero();
	if (Color::is_onto(blend_method))
		bounds = ra;
	else
	if (ra.valid() && rb.valid())
		set_union(bounds, ra, rb);
	else
	if (ra.valid())
		bounds = ra;
	else
	if (rb.valid())
		bounds = rb;
	return bounds;
}

VectorInt
TaskBlend::get_offset_a() const
{
	if (!sub_task_a()) return VectorInt::zero();
	Vector offset = (sub_task_a()->source_rect.get_min() - source_rect.get_min()).multiply_coords(get_pixels_per_unit());
	return VectorInt((int)round(offset[0]), (int)round(offset[1])) - sub_task_a()->target_rect.get_min();
}

VectorInt
TaskBlend::get_offset_b() const
{
	if (!sub_task_b()) return VectorInt::zero();
	Vector offset = (sub_task_b()->source_rect.get_min() - source_rect.get_min()).multiply_coords(get_pixels_per_unit());
	return VectorInt((int)round(offset[0]), (int)round(offset[1])) - sub_task_b()->target_rect.get_min();
}

/* === E N T R Y P O I N T ================================================= */
