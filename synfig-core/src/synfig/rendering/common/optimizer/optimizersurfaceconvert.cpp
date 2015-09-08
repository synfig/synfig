/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizersurfaceconvert.cpp
**	\brief OptimizerSurfaceConvert
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

#include "optimizersurfaceconvert.h"

#include "../task/tasksurfaceconvert.h"
#include "../../software/surfacesw.h"
#include "../../software/task/tasksw.h"
#include "../../opengl/surfacegl.h"
#include "../../opengl/task/taskgl.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
OptimizerSurfaceConvert::run(const RunParams& params) const
{
	if (params.task)
	{
		bool sw = (bool)TaskSW::Handle::cast_dynamic(params.task);
		bool gl = (bool)TaskGL::Handle::cast_dynamic(params.task);
		if (sw || gl)
		{
			// Insert TaskSurfaceConvert when surfaces in task not native
			Task::Handle task = params.task;

			for(Task::List::iterator i = task->sub_tasks.begin(); i != task->sub_tasks.end(); ++i)
			{
				if (*i && (*i)->target_surface
				 && ( (sw && !SurfaceSW::Handle::cast_dynamic((*i)->target_surface))
				   || (gl && !SurfaceGL::Handle::cast_dynamic((*i)->target_surface)) ))
				{
					if (task == params.task) {
						task = params.task->clone();
						i = task->sub_tasks.begin() + (i - params.task->sub_tasks.begin());
					}
					TaskSurfaceConvert::Handle surface_convert(new TaskSurfaceConvert());
					surface_convert->sub_task() = (*i)->clone();
					if (sw) surface_convert->target_surface = new SurfaceSW();
					if (gl) surface_convert->target_surface = new SurfaceGL();
					surface_convert->target_surface->set_size( surface_convert->sub_task()->target_surface->get_size() );
					*i = surface_convert;
				}
			}

			if (task->target_surface
			 && ( (sw && !SurfaceSW::Handle::cast_dynamic(task->target_surface))
			   || (gl && !SurfaceGL::Handle::cast_dynamic(task->target_surface)) ))
			{
				if (task == params.task) task = params.task->clone();
				TaskSurfaceConvert::Handle surface_convert(new TaskSurfaceConvert());
				surface_convert->sub_task() = task;
				surface_convert->target_surface = task->target_surface;
				if (sw) task->target_surface = new SurfaceSW();
				if (gl) task->target_surface = new SurfaceGL();
				task->target_surface->set_size( surface_convert->target_surface->get_size() );
				task = surface_convert;
			}

			if (task != params.task)
			{
				params.out_task = task;
				return true;
			}
		}
		else
		if (TaskSurfaceConvert::Handle surface_convert = TaskSurfaceConvert::Handle::cast_dynamic(params.task))
		{
			// Remove unneeded TaskSurfaceConvert
			if ( !surface_convert->target_surface
			  || !surface_convert->sub_task()
			  || !surface_convert->sub_task()->target_surface
			  || ( SurfaceSW::Handle::cast_dynamic(surface_convert->target_surface)
			    && SurfaceSW::Handle::cast_dynamic(surface_convert->sub_task()->target_surface))
			  || ( SurfaceGL::Handle::cast_dynamic(surface_convert->target_surface)
			    && SurfaceGL::Handle::cast_dynamic(surface_convert->sub_task()->target_surface)) )
			{
				params.out_task = surface_convert->sub_task();
				return true;
			}
		}
	}
	return false;
}

/* === E N T R Y P O I N T ================================================= */
