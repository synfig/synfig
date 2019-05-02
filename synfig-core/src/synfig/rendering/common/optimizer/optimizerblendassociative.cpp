/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerblendassociative.cpp
**	\brief OptimizerBlendAssociative
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

#include <synfig/general.h>
#include <synfig/localization.h>

#include "optimizerblendassociative.h"

#include "../task/taskblend.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

OptimizerBlendAssociative::OptimizerBlendAssociative()
{
	category_id = CATEGORY_ID_SPECIALIZED;
	depends_from = CATEGORY_COORDS;
	mode = MODE_REPEAT_PARENT;
	deep_first = true;
	for_task = true;
}

void
OptimizerBlendAssociative::run(const RunParams& params) const
{
	//
	// in some cases we can do this transformation:
	//   blend1(a, blend0(b, c)) -> blend1(blend0(a, b), c)
	// result chain may be converted to list
	//
	// as result actually we will do following:
	//   blend1(a, list(...)) -> list(a, ...)
	// if each task in list suits for this optimization
	//

	int max_count = 50;

	if ( !params.parent
	  || !params.parent->ref_task.type_is<TaskList>()
	  || (int)params.parent->ref_task->sub_tasks.size() >= max_count )
		return;

	max_count -= params.parent->ref_task->sub_tasks.size();

	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && ( !blend->sub_task_a()
		|| blend->sub_task_a()->target_surface == blend->target_surface )
	  && ((1 << blend->blend_method) & Color::BLEND_METHODS_ASSOCIATIVE)
	  && approximate_equal_lp(blend->amount, ColorReal(1.0)) )
	{
		if (TaskList::Handle list = TaskList::Handle::cast_dynamic(blend->sub_task_b()))
		if ((int)list->sub_tasks.size() <= max_count)
		{
			// check each task in list
			bool fix_list = false;
			for(Task::List::iterator i = list->sub_tasks.begin(); i != list->sub_tasks.end(); ++i)
			{
				if (!*i) { fix_list = true; continue; }
				if (TaskBlend::Handle sub_blend = TaskBlend::Handle::cast_dynamic(*i))
					if ( sub_blend->blend_method == blend->blend_method )
						continue;
				if (TaskInterfaceBlendToTarget *interface = i->type_pointer<TaskInterfaceBlendToTarget>()) {
					if (!interface->blend) { fix_list = true; continue; }
					if (interface->blend_method == blend->blend_method )
						continue;
				}
				return;
			}

			Task::Handle new_list = replace_target(blend, list);

			// enable blending for TaskInterfaceBlendToTarget
			if (fix_list)
			{
				if (new_list == list) new_list = list->clone();
				new_list->sub_tasks.insert(new_list->sub_tasks.begin(), blend->sub_task_a());
				for(Task::List::iterator i = new_list->sub_tasks.begin(); i != new_list->sub_tasks.end();)
				{
					if (!*i) { i = list->sub_tasks.erase(i); continue; }
					if (!i->type_is<TaskBlend>())
						if (TaskInterfaceBlendToTarget *interface = i->type_pointer<TaskInterfaceBlendToTarget>())
							if (!interface->blend) {
								*i = (*i)->clone();
								interface = i->type_pointer<TaskInterfaceBlendToTarget>();
								interface->blend = true;
								interface->blend_method = blend->blend_method;
								interface->amount = 1.0;
								Task::Handle surface = new TaskSurface();
								surface->assign_target(**i);
								interface->target_subtask() = surface;
								interface->on_target_set_as_source();
							}
					++i;
				}
			}

			// insert sub-task A in front
			if ( blend->sub_task_a()
			 && !blend->sub_task_a().type_is<TaskSurface>() )
			{
				if (new_list == list) new_list = list->clone();
				new_list->sub_tasks.insert(new_list->sub_tasks.begin(), blend->sub_task_a());
			}

			apply(params, new_list);
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
