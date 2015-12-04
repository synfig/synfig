/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizertransformationaffine.cpp
**	\brief OptimizerTransformationAffine
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

#include "optimizertransformationaffine.h"

#include "../../primitive/affinetransformation.h"
#include "../task/taskblend.h"
#include "../task/tasksolid.h"
#include "../task/tasktransformation.h"
#include "../task/tasktransformableaffine.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerTransformationAffine::replace(Task::Handle &dest, const Task::Handle &src, bool clonned) const
{
	if (src && dest && dest->valid_target() && !src->valid_target())
	{
		Task::Handle task = clonned ? src : src->clone();
		task->target_surface = dest->target_surface;
		task->init_target_rect(dest->get_target_rect(), dest->get_source_rect_lt(), dest->get_source_rect_rb());
		dest = task;
		return;
	}
	dest = src;
}

void
OptimizerTransformationAffine::recursive(Task::Handle &ref_task, const Matrix &matrix) const
{
	if (!ref_task)
		return;

	if (TaskTransformation::Handle transformation = TaskTransformation::Handle::cast_dynamic(ref_task))
	{
		if (AffineTransformation::Handle affine_transformation = AffineTransformation::Handle::cast_dynamic(transformation->transformation))
		{
			// skip identity transfromation
			if (affine_transformation->matrix.is_identity())
			{
				replace(ref_task, transformation->sub_task());
				recursive(ref_task, matrix);
				return;
			}

			// transformation of null-task is meaningless, remove transformation and return
			if (!transformation->sub_task())
			{
				replace(ref_task, Task::Handle());
				return;
			}

			// transformation of TaskSolid is meaningless, skip transformation and continue branch
			if (transformation->sub_task().type_is<TaskSolid>())
			{
				replace(ref_task, transformation->sub_task());
				recursive(ref_task, Matrix());
				return;
			}

			// apply affine transformation to sub-tasks of blend
			if (TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(transformation->sub_task()))
			{
				replace(ref_task, blend);
				recursive(ref_task, affine_transformation->matrix * matrix);
				return;
			}

			// apply affine transformation to transformable sub-task
			if (transformation->sub_task().type_is<TaskTransformableAffine>())
			{
				replace(ref_task, transformation->sub_task());
				recursive(ref_task, affine_transformation->matrix * matrix);
				return;
			}

			// apply affine transformation to other affine transformation
			if (TaskTransformation::Handle sub_transformation = TaskTransformation::Handle::cast_dynamic(transformation->sub_task()))
			{
				if (AffineTransformation::Handle::cast_dynamic(sub_transformation->transformation))
				{
					replace(ref_task, transformation->sub_task());
					recursive(ref_task, affine_transformation->matrix*matrix);
					return;
				}
			}

			// use current transformation task without modifications
			if (matrix.is_identity())
			{
				Task::Handle sub_task = transformation->sub_task();
				recursive(sub_task, Matrix());
				if (sub_task != transformation->sub_task())
				{
					transformation = TaskTransformation::Handle::cast_dynamic(transformation->clone());
					transformation->sub_task() = sub_task;
					replace(ref_task, transformation, true);
				}
				return;
			}

			// take matrix and remove current transformation, it will recreated with new matrix in recursive-call
			replace(ref_task, transformation->sub_task());
			recursive(ref_task, affine_transformation->matrix * matrix);
			return;
		}
	}

	Matrix m = matrix;
	if (ref_task.type_is<TaskSolid>())
		m.set_identity();

	if ( !m.is_identity() )
	{
		if ( TaskBlend::Handle::cast_dynamic(ref_task) )
		{
			bool task_clonned = false;
			for(Task::List::iterator i = ref_task->sub_tasks.begin(); i != ref_task->sub_tasks.end(); ++i)
			{
				if (*i)
				{
					if (!task_clonned)
					{
						Task::Handle sub_task = *i;
						recursive(sub_task, m);
						if (sub_task != *i)
						{
							int index = i - ref_task->sub_tasks.begin();
							replace(ref_task, ref_task->clone(), true);
							i = ref_task->sub_tasks.begin() + index;
							*i = sub_task;
							task_clonned = true;
						}
					}
					else
					{
						recursive(*i, m);
					}
				}
			}
			return;
		}

		// apply affine transformation to transformable sub-task
		if (ref_task.type_is<TaskTransformableAffine>())
		{
			replace(ref_task, ref_task->clone(), true);
			TaskTransformableAffine *transformable_affine = ref_task.type_pointer<TaskTransformableAffine>();
			transformable_affine->transformation *= matrix;
			recursive(ref_task, Matrix());
			return;
		}

		// transformation task is required here, create it
		TaskTransformation::Handle transformation(new TaskTransformation());
		AffineTransformation::Handle affine_transformation = new AffineTransformation();
		affine_transformation->matrix = matrix;
		transformation->transformation = affine_transformation;
		transformation->sub_task() = ref_task;
		replace(ref_task, transformation, true);
		recursive(transformation->sub_task(), Matrix());
		return;
	}

	// recursive call for all sub-tasks
	bool task_clonned = false;
	for(Task::List::iterator i = ref_task->sub_tasks.begin(); i != ref_task->sub_tasks.end(); ++i)
	{
		if (*i)
		{
			if (!task_clonned)
			{
				Task::Handle sub_task = *i;
				recursive(sub_task, Matrix());
				if (sub_task != *i)
				{
					int index = i - ref_task->sub_tasks.begin();
					replace(ref_task, ref_task->clone(), true);
					i = ref_task->sub_tasks.begin() + index;
					*i = sub_task;
					task_clonned = true;
				}
			}
			else
			{
				recursive(*i, Matrix());
			}
		}
	}
	return;
}


void
OptimizerTransformationAffine::run(const RunParams& params) const
{
	Task::Handle task = params.ref_task;
	recursive(task, Matrix());
	if (task != params.ref_task)
		apply(params, task);
}

/* === E N T R Y P O I N T ================================================= */
