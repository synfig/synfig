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
#ifdef WITH_OPENGL
#include "../../opengl/surfacegl.h"
#include "../../opengl/task/taskgl.h"
#endif

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerSurfaceConvert::run(const RunParams& params) const
{
#ifdef WITH_OPENGL
	if (params.ref_task->target_surface)
	{
		if (TaskSurfaceConvert::Handle surface_convert = TaskSurfaceConvert::Handle::cast_dynamic(params.ref_task))
		{
			// remove unneeded TaskSurfaceConvert

			if (surface_convert->sub_task() && surface_convert->sub_task()->target_surface)
			{
				if ( ( SurfaceSW::Handle::cast_dynamic(surface_convert->target_surface)
				    && SurfaceSW::Handle::cast_dynamic(surface_convert->sub_task()->target_surface) )
				  || ( SurfaceGL::Handle::cast_dynamic(surface_convert->target_surface)
			        && SurfaceGL::Handle::cast_dynamic(surface_convert->sub_task()->target_surface) ) )
				{
					apply(params, surface_convert->sub_task());
				}
			}
		}
		else
		{
			// add surface TaskSurfaceConvert

			bool target_sw = SurfaceSW::Handle::cast_dynamic(params.ref_task->target_surface);
			bool target_gl = SurfaceGL::Handle::cast_dynamic(params.ref_task->target_surface);

			bool sw = params.ref_task.type_is<TaskSW>();
			bool gl = params.ref_task.type_is<TaskGL>();
			if ( (sw && !target_sw) || (gl && !target_gl) )
			{
				Task::Handle task = params.ref_task;
				TaskSurfaceConvert::Handle surface_convert(new TaskSurfaceConvert());
				surface_convert->sub_task() = task;
				surface_convert->target_surface = surface_convert->sub_task()->target_surface;
				if (sw) task->target_surface = new SurfaceSW();
				if (gl) task->target_surface = new SurfaceGL();
				task->target_surface->is_temporary = true;
				task->target_surface->set_size( surface_convert->target_surface->get_size() );
				apply(params, surface_convert);
			}

			if (params.parent && params.parent->ref_task)
			{
				bool parent_sw = params.parent->ref_task.type_is<TaskSW>();
				bool parent_gl = params.parent->ref_task.type_is<TaskGL>();
				if ( (parent_sw && !target_sw) || (parent_gl && !target_gl) )
				{
					TaskSurfaceConvert::Handle surface_convert(new TaskSurfaceConvert());
					surface_convert->sub_task() = params.ref_task;
					if (parent_sw) surface_convert->target_surface = new SurfaceSW();
					if (parent_gl) surface_convert->target_surface = new SurfaceGL();
					surface_convert->target_surface->is_temporary = true;
					surface_convert->target_surface->set_size( params.ref_task->target_surface->get_size() );
					apply(params, surface_convert);
				}
			}
		}
	}
#endif
}

/* === E N T R Y P O I N T ================================================= */
