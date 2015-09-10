/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizersurface.cpp
**	\brief OptimizerSurface
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

#include "optimizersurface.h"

#include "../../software/task/tasksw.h"
#include "../../software/surfacesw.h"
#include "../../opengl/task/taskgl.h"
#include "../../opengl/surfacegl.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
OptimizerSurface::run(const RunParams& params) const
{
	if ( params.task
	  && params.task->target_surface
	  && !params.task->target_surface->empty())
	{
		bool sw = params.task.type_is<TaskSW>();
		bool gl = params.task.type_is<TaskGL>();
		if (sw || gl)
		{
			// Create surfaces when subtasks have no target_surface
			Task::Handle task = params.task;
			for(Task::List::iterator i = task->sub_tasks.begin(); i != task->sub_tasks.end(); ++i)
			{
				if (*i && (!(*i)->target_surface || (*i)->target_surface->empty()))
				{
					if (task == params.task)
					{
						task = params.task->clone();
						i = task->sub_tasks.begin() + (i - params.task->sub_tasks.begin());
					}
					*i = (*i)->clone();

					if (!(*i)->target_surface)
					{
						if (sw) (*i)->target_surface = new SurfaceSW();
						if (gl) (*i)->target_surface = new SurfaceGL();
						(*i)->target_surface->is_temporary = true;
					}
					(*i)->target_surface->set_size( task->target_surface->get_size() );
					(*i)->rect_lt = task->rect_lt;
					(*i)->rect_rb = task->rect_rb;
				}
			}

			if (task != params.task)
			{
				params.out_task = task;
				return true;
			}
		}
	}
	return false;
}

/* === E N T R Y P O I N T ================================================= */
