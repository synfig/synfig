/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/tasktransformation.cpp
**	\brief TaskTransformation
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "tasktransformation.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Task::Token TaskTransformation::token(
	DescAbstract<TaskTransformation>("Transformation") );
SYNFIG_EXPORT Task::Token TaskTransformationAffine::token(
	DescAbstract<TaskTransformationAffine, TaskTransformation>("TransformationAffine") );


TaskTransformation::TaskTransformation():
	interpolation(Color::INTERPOLATION_CUBIC),
	supersample(Vector(1.0, 1.0)) { }


bool
TaskTransformation::is_simple() const {
	return interpolation != Color::INTERPOLATION_NEAREST
		&& supersample.is_equal_to(Vector(1.0, 1.0));
}

int
TaskTransformation::get_pass_subtask_index() const
{
	if (!get_transformation())
		return PASSTO_NO_TASK;
	for(Task::List::const_iterator i = sub_tasks.begin(); i != sub_tasks.end(); ++i)
		if (*i) return PASSTO_THIS_TASK;
	return PASSTO_NO_TASK;
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
	if (!sub_task())
		{ trunc_to_zero(); return; }

	if ( is_valid_coords()
	  && approximate_greater(supersample[0], Real(0))
	  && approximate_greater(supersample[1], Real(0)) )
	{
		if (Transformation::Handle transformation = get_transformation())
		{
			if (Transformation::Handle back_transformation = transformation->create_inverted())
			{
				Transformation::DiscreteBounds discrete_bounds =
					Transformation::make_discrete_bounds(
						back_transformation->transform_bounds(
							source_rect,
							get_pixels_per_unit().multiply_coords(supersample) ));
				if (discrete_bounds.is_valid())
				{
					sub_task()->set_coords(discrete_bounds.rect, discrete_bounds.size);
					return;
				}
			}
		}
	}

	sub_task()->set_coords_zero();
	trunc_to_zero();
}


int
TaskTransformationAffine::get_pass_subtask_index() const
{
	if (is_simple() && transformation->matrix.is_identity())
		return 0;
	return TaskTransformation::get_pass_subtask_index();
}

/* === E N T R Y P O I N T ================================================= */
