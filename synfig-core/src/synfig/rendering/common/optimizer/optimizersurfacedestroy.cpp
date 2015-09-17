/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizersurfacedestroy.cpp
**	\brief OptimizerSurfaceDestroy
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

#include "optimizersurfacedestroy.h"

#include "../task/tasksurfacedestroy.h"


#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerSurfaceDestroy::insert_task(
	std::set<Surface::Handle> &destroyed_surfaces,
	const RunParams& params,
	Task::List::reverse_iterator &ri,
	const Task::Handle &task ) const
{
	if ( task
	  && task->target_surface
	  && !task->target_surface->is_created()
	  && !destroyed_surfaces.count(task->target_surface) )
	{
		destroyed_surfaces.insert(task->target_surface);
		TaskSurfaceDestroy::Handle surface_destroy = new TaskSurfaceDestroy();
		surface_destroy->target_surface = task->target_surface;
		Task::List::iterator i(ri.base());
		i = params.list.insert(i, surface_destroy);
		ri = Task::List::reverse_iterator(i);
		apply(params);
	}
}

void
OptimizerSurfaceDestroy::run(const RunParams& params) const
{
	std::set<Surface::Handle> destroyed_surfaces;
	for(Task::List::reverse_iterator ri = params.list.rbegin(); ri != params.list.rend();)
	{
		if (*ri)
		{
			if (TaskSurfaceDestroy::Handle::cast_dynamic(*ri))
			{
				if (destroyed_surfaces.count((*ri)->target_surface))
				{
					Task::List::iterator i(ri.base());
					i = params.list.erase(i);
					ri = Task::List::reverse_iterator(i);
					continue;
				}
				destroyed_surfaces.insert((*ri)->target_surface);
			}
			else
			{
				for(std::vector<Task::Handle>::const_iterator j = (*ri)->sub_tasks.begin(); j != (*ri)->sub_tasks.end(); ++j)
					insert_task(destroyed_surfaces, params, ri, *j);
				insert_task(destroyed_surfaces, params, ri, *ri);
			}
		}
		++ri;
	}
}

/* === E N T R Y P O I N T ================================================= */
