/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizertransformation.cpp
**	\brief OptimizerTransformation
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

#include "optimizertransformation.h"

#include "../../primitive/affinetransformation.h"
#include "../task/tasktransformation.h"
#include "../task/tasktransformableaffine.h"
#include "../task/tasksolid.h"
#include "../task/taskmesh.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
OptimizerTransformation::can_optimize(const Task::Handle &sub_task)
{
	if (!sub_task)
		return true;
	if (sub_task.type_is<TaskSolid>())
		return true;
	if (sub_task.type_is<TaskTransformableAffine>())
		return true;
	if (TaskTransformation::Handle transformation = TaskTransformation::Handle::cast_dynamic(sub_task))
		if (AffineTransformation::Handle::cast_dynamic(transformation->transformation))
			return true;
	return false;
}

void
OptimizerTransformation::calc_unoptimized_blend_brunches(int &ref_count, const Task::Handle &blend_sub_task)
{
	if (ref_count > 1)
		return;
	if (can_optimize(blend_sub_task))
		return;
	if (TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(blend_sub_task))
	{
		calc_unoptimized_blend_brunches(ref_count, blend->sub_task_a());
		calc_unoptimized_blend_brunches(ref_count, blend->sub_task_b());
		return;
	}
	++ref_count;
}

void
OptimizerTransformation::run(const RunParams& params) const
{
	// TODO: Optimize transformation to transformation
	if (TaskTransformation::Handle transformation = TaskTransformation::Handle::cast_dynamic(params.ref_task))
	{
		// transformation of TaskSolid or null-task is meaningless
		if ( !transformation->sub_task()
		  || transformation->sub_task().type_is<TaskSolid>())
		{
			apply(params, transformation->sub_task());
			return;
		}

		// optimize affine transformation
		if (AffineTransformation::Handle affine_transformation = AffineTransformation::Handle::cast_dynamic(transformation->transformation))
		{
			if (affine_transformation->matrix.is_identity())
			{
				apply(params, transformation->sub_task());
				return;
			}
			else
			if (transformation->sub_task().type_is<TaskTransformableAffine>())
			{
				// apply affine transformation to sub-task
				Task::Handle task = transformation->sub_task()->clone();
				TaskTransformableAffine *transformable_affine = task.type_pointer<TaskTransformableAffine>();
				transformable_affine->transformation *=
					affine_transformation->matrix;
				apply(params, task);
				return;
			}
			else
			if (TaskTransformation::Handle sub_transformation = TaskTransformation::Handle::cast_dynamic(transformation->sub_task()))
			{
				// optimize affine transformation of affine transformation
				if (AffineTransformation::Handle sub_affine_transformation = AffineTransformation::Handle::cast_dynamic(sub_transformation->transformation))
				{
					transformation = TaskTransformation::Handle::cast_dynamic(transformation->clone());
					AffineTransformation::Handle new_affine_transformation = new AffineTransformation();
					new_affine_transformation->matrix =
						sub_affine_transformation->matrix
					  * affine_transformation->matrix;
					transformation->transformation = new_affine_transformation;
					transformation->sub_task() = sub_transformation->sub_task();

					apply(params, transformation);
					return;
				}
			}
			else
			if (TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(transformation->sub_task()))
			{
				// optimize transfromation of TaskBlend
				//int count = 0;
				//calc_unoptimized_blend_brunches(count, blend->sub_task_a());
				//calc_unoptimized_blend_brunches(count, blend->sub_task_b());
				//if (count < 2)
				//{
					blend = TaskBlend::Handle::cast_dynamic(blend->clone());
					TaskTransformation::Handle transformation_a = TaskTransformation::Handle::cast_dynamic(transformation->clone());
					transformation_a->sub_task() = blend->sub_task_a();
					blend->sub_task_a() = transformation_a;
					TaskTransformation::Handle transformation_b = TaskTransformation::Handle::cast_dynamic(transformation->clone());
					transformation_b->sub_task() = blend->sub_task_b();
					blend->sub_task_b() = transformation_b;
					apply(params, blend);
					return;
				//}
			}
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
