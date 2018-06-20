/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/tasktransformation.cpp
**	\brief TaskTransformation
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

#include "tasktransformation.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Task::Token TaskTransformation::token(
	DescAbstract<TaskTransformation>("TaskTransformation") );
Task::Token TaskTransformationAffine::token(
	DescAbstract<TaskTransformationAffine, TaskTransformation>("TaskTransformationAffine") );


TaskTransformation::TaskTransformation():
	interpolation(Color::INTERPOLATION_CUBIC),
	supersample(Vector(1.0, 1.0)) { }


bool
TaskTransformation::is_simple() const {
	return interpolation != Color::INTERPOLATION_NEAREST
		&& supersample.is_equal_to(Vector(1.0, 1.0));
}

Rect
TaskTransformation::calc_bounds() const
{
	if (!sub_task())
		return Rect();
	Transformation::Handle transformation = get_transformation();
	if (!transformation)
		return Rect();
	Rect bounds = sub_task()->get_bounds();
	if (!bounds.is_valid())
		return Rect();

	// transform_bounds() cannot transform infinity,
	// because transform_bounds() calculates bounds with resolution
	if (bounds.is_full_infinite())
		return bounds;

	return transformation->transform_bounds(bounds).rect;
}

void
TaskTransformation::set_coords_sub_tasks()
{
	const int border = 4;

	if (!sub_task())
		{ trunc_to_zero(); return; }

	if ( is_valid_coords()
	  && approximate_greater(supersample[0], 0.0)
	  && approximate_greater(supersample[1], 0.0) )
	{
		if (Transformation::Handle transformation = get_transformation())
		{
			if (Transformation::Handle back_transformation = transformation->create_inverted())
			{
				Transformation::Bounds bounds =
					back_transformation->transform_bounds(
						source_rect, get_pixels_per_unit().multiply_coords(supersample) );
				if (bounds.is_valid())
				{
					// add some pixels to border for draw valid and antialiased transformed edges
					Vector size_real = bounds.resolution.multiply_coords( bounds.rect.get_size() );
					VectorInt size(	2*border + ceil(size_real[0]),
							        2*border + ceil(size_real[1]) );
					Vector extra( 0.5*(size[0] - size_real[0])/bounds.resolution[0],
							      0.5*(size[1] - size_real[1])/bounds.resolution[1] );
					Rect rect = bounds.rect;
					rect.minx -= extra[0];
					rect.miny -= extra[1];
					rect.maxx += extra[0];
					rect.maxy += extra[1];
					sub_task()->set_coords(rect, size);
					return;
				}
			}
		}
	}

	sub_task()->set_coords_zero();
	trunc_to_zero();
}

/* === E N T R Y P O I N T ================================================= */
