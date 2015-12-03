/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerlist.cpp
**	\brief OptimizerList
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

#include "optimizerlist.h"

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
OptimizerList::clone_list(const RunParams &params, Task::List::iterator &i, TaskList::Handle &list) const
{
	if (params.ref_task == list)
	{
		int index = i - list->sub_tasks.begin();
		list = TaskList::Handle::cast_dynamic(list->clone());
		i = list->sub_tasks.begin() + index;
		apply(params, list);
	}
}

void
OptimizerList::run(const RunParams& params) const
{
	if (TaskList::Handle list = TaskList::Handle::cast_dynamic(params.ref_task))
	{
		for(Task::List::iterator i = list->sub_tasks.begin(); i != list->sub_tasks.end();)
		{
			if (!(*i) || !(*i)->valid_target() || i->type_is<TaskSurface>() || i->type_is<TaskSurfaceEmpty>())
			{
				clone_list(params, i, list);
				i = list->sub_tasks.erase(i);
				continue;
			}

			if (TaskList::Handle sub_list = TaskList::Handle::cast_dynamic(*i))
			{
				clone_list(params, i, list);
				i = list->sub_tasks.erase(i);
				int index = i - list->sub_tasks.begin();
				list->sub_tasks.insert(i, sub_list->sub_tasks.begin(), sub_list->sub_tasks.end());
				i = list->sub_tasks.begin() + index;
				continue;
			}

			++i;
		}

		if (list->sub_tasks.size() == 1)
			apply(params, list->sub_tasks[0]);
	}
}

/* === E N T R Y P O I N T ================================================= */
