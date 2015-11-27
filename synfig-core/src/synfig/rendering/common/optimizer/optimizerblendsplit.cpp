/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerblendsplit.cpp
**	\brief OptimizerBlendSplit
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

#include <synfig/general.h>
#include <synfig/localization.h>

#include "optimizerblendsplit.h"

#include "../task/taskblend.h"
#include "../task/tasklist.h"
#include "../task/tasksurface.h"
#include "../task/tasksurfaceempty.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerBlendSplit::run(const RunParams& params) const
{
	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->target_surface
	  && blend->sub_task_a()
	  && blend->sub_task_a()->target_surface == blend->target_surface
	  && (blend->sub_task_a().type_is<TaskSurface>() || blend->sub_task_a().type_is<TaskSurfaceEmpty>())
	  && blend->sub_task_b()
	  && blend->sub_task_b()->target_surface
	  && !Color::is_straight(blend->blend_method) )
	{
		if (TaskList::Handle list_b = TaskList::Handle::cast_dynamic(blend->sub_task_b()))
		{
			// try to find dedicated groups
			std::vector<RectInt> groups;
			for(Task::List::const_iterator i = list_b->sub_tasks.begin(); i != list_b->sub_tasks.end(); ++i)
				if (*i && (*i)->target_rect.valid())
					groups.push_back((*i)->target_rect);

			bool retry = true;
			while(!retry)
			{
				retry = false;
				for(int i = 0; i < (int)groups.size(); ++i)
				{
					for(int j = i+1; j < (int)groups.size(); ++j)
					{
						if (etl::intersect(groups[i], groups[j]))
						{
							etl::set_union(groups[i], groups[i], groups[j]);
							groups.erase(groups.begin() + j);
							--j;
							retry = true;
						}
					}
				}
			}

			// split task
			if (groups.size() > 1)
			{
				// create list
				TaskList::Handle list;
				list = new TaskList();
				assign(list, Task::Handle(blend));
				list->sub_tasks.clear();

				// fill list
				for(int j = 0; j < (int)groups.size(); ++j)
				{
					// create sub-blend
					TaskList::Handle sub_list_b = TaskList::Handle::cast_dynamic(blend->sub_task_b()->clone());
					sub_list_b->sub_tasks.clear();
					sub_list_b->target_rect = groups[j];

					RectInt rect = groups[j]
								 + blend->target_rect.get_min()
								 + blend->offset_b;
					TaskBlend::Handle sub_blend = TaskBlend::Handle::cast_dynamic(blend->clone());
					sub_blend->target_rect = rect;
					sub_blend->sub_task_a() = new TaskSurface();
					assign(sub_blend->sub_task_a(), blend->sub_task_a());
					etl::set_intersect(sub_blend->sub_task_a()->target_rect, blend->sub_task_a()->target_rect, rect);
					sub_blend->sub_task_b() = sub_list_b;
					sub_blend->offset_a = -rect.get_min();
					sub_blend->offset_b += blend->target_rect.get_min() - rect.get_min();

					list->sub_tasks.push_back(sub_blend);

					// fill list-b of sub-blend
					for(Task::List::const_iterator i = list_b->sub_tasks.begin(); i != list_b->sub_tasks.end(); ++i)
						if (*i && (*i)->target_rect.valid() && etl::intersect(groups[j], (*i)->target_rect))
							sub_list_b->sub_tasks.push_back(*i);

					// optimization for list with single task
					if (sub_list_b->sub_tasks.size() == 1)
						sub_blend->sub_task_b() = sub_list_b->sub_tasks[0];
				}

				apply(params, list);
			}
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
