/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizertransformation.cpp
**	\brief OptimizerTransformation
**
**	$Id$
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

#include "optimizertransformation.h"

#include "../task/tasktransformation.h"
#include "../../primitive/transformationaffine.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


OptimizerTransformation::OptimizerTransformation()
{
	category_id = CATEGORY_ID_BEGIN;
	mode = MODE_REPEAT_LAST | MODE_RECURSIVE;
	for_task = true;
}


void
OptimizerTransformation::run(const RunParams& params) const
{
	TaskTransformation::Handle transformation = TaskTransformation::Handle::cast_dynamic(params.ref_task);
	if (!transformation || !transformation->is_simple()) return;

	// no trasgormation
	if (!transformation->get_transformation())
		{ apply(params, Task::Handle()); return; }
	
	const Task::List &sub_tasks = transformation->sub_tasks;
	int count = 0;
	for(Task::List::const_iterator i = sub_tasks.begin(); i != sub_tasks.end(); ++i)
		if (*i) ++count;
	if (!count)
		{ apply(params, Task::Handle()); return; }

	// transformation of none in none
	Task::Handle sub_task = transformation->sub_task();
	if (!sub_task || count > 1)
		return;
	
	// transformation of solid is solid
	if (TaskInterfaceConstant *constant = sub_task.type_pointer<TaskInterfaceConstant>())
	{
		if (constant->is_constant())
		{
			sub_task = sub_task->clone();
			sub_task->assign_target(*transformation);
			apply(params, sub_task);
			return;
		}
	}
	
	// merge into sub-task
	if (TaskInterfaceTransformation *interface = sub_task.type_pointer<TaskInterfaceTransformation>())
	{
		if ( interface->get_transformation()
		  && interface->get_transformation()->can_merge_outer( transformation->get_transformation() ) )
		{
			sub_task = sub_task->clone();
			sub_task->assign_target(*transformation);

			interface = sub_task.type_pointer<TaskInterfaceTransformation>();
			assert( interface
			     && interface->get_transformation()->can_merge_outer( transformation->get_transformation()) );
			interface->get_transformation()->merge_outer( transformation->get_transformation() );

			apply(params, sub_task);
			return;
		}
	}

	// merge sub-task into current task
	if (TaskTransformation::Handle sub_transformation = TaskTransformation::Handle::cast_dynamic(sub_task))
	{
		if ( sub_transformation->is_simple()
		  && sub_transformation->sub_task()
		  && !sub_transformation->target_surface // exclude tasks with prerendered source
		  && transformation->get_transformation()->can_merge_inner(sub_transformation->get_transformation()) )
		{
			transformation = TaskTransformation::Handle::cast_dynamic(transformation->clone());
			// recheck ability to merge after clone
			assert( transformation->get_transformation()->can_merge_inner( transformation->get_transformation()) );
			transformation->get_transformation()->merge_inner( sub_transformation->get_transformation() );
			transformation->sub_task() = sub_transformation->sub_task();
			apply(params, transformation);
			return;
		}
	}
	
	// fall deeper in tree to make a chance to merge with others (for affine only)
	if ( transformation->get_transformation().type_is<TransformationAffine>() )
	{
		if ( sub_task.type_is<TaskInterfaceTransformationPass>() )
		{
			sub_task = sub_task->clone();
			sub_task->assign_target(*transformation);
			for(Task::List::iterator i = sub_task->sub_tasks.begin(); i != sub_task->sub_tasks.end(); ++i)
				if (*i) {
					Task::Handle t = transformation->clone();
					t->assign_target(**i);
					t->sub_task(0) = *i;
					*i = t;
				}
			apply(params, sub_task);
			return;
		}
	}
}


/* === E N T R Y P O I N T ================================================= */
