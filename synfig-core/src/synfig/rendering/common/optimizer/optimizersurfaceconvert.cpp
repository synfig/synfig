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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "optimizersurfaceconvert.h"

#include "../task/tasksurfaceconvert.h"

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
	if (params.ref_task->target_surface)
	{
		// remove unneeded TaskSurfaceConvert
		if (TaskSurfaceConvert::Handle surface_convert = TaskSurfaceConvert::Handle::cast_dynamic(params.ref_task))
		{
			if (params.parent && params.parent->ref_task && surface_convert->sub_task())
			{
				if (TackCapabilityInterface *capability = params.parent->ref_task.type_pointer<TackCapabilityInterface>())
					if (capability->is_supported_source(surface_convert->sub_task()->target_surface))
						{ apply(params, surface_convert->sub_task()); return; }
				if (TackCapabilityInterface *capability = surface_convert->sub_task().type_pointer<TackCapabilityInterface>())
					if (capability->is_supported_target(surface_convert->target_surface))
						{ apply(params, surface_convert->sub_task()); return; }
			}
		}
		else
		// add TaskSurfaceConvert for target
		if (TackCapabilityInterface *capability = params.ref_task.type_pointer<TackCapabilityInterface>())
		{
			if (!capability->is_supported_target(params.ref_task->target_surface))
			{
				Task::Handle task = params.ref_task;
				TaskSurfaceConvert::Handle surface_convert;
				init_and_assign(surface_convert, task);
				surface_convert->sub_tasks.clear();
				surface_convert->sub_task() = task;
				surface_convert->target_surface = task->target_surface;
				task->target_surface = capability->create_supported_target();
				task->target_surface->is_temporary = true;
				task->target_surface->set_size( surface_convert->target_surface->get_size() );
				apply(params, surface_convert);
			}
		}
		else
		// add TaskSurfaceConvert for source
		if (params.parent && params.parent->ref_task)
		{
			if (TackCapabilityInterface *capability = params.parent->ref_task.type_pointer<TackCapabilityInterface>())
			{
				if (!capability->is_supported_source(params.ref_task->target_surface))
				{
					TaskSurfaceConvert::Handle surface_convert(new TaskSurfaceConvert());
					init_and_assign(surface_convert, params.ref_task);
					surface_convert->sub_tasks.clear();
					surface_convert->sub_task() = params.ref_task;
					surface_convert->target_surface = capability->create_supported_source();
					surface_convert->target_surface->is_temporary = true;
					surface_convert->target_surface->set_size( params.ref_task->target_surface->get_size() );
					apply(params, surface_convert);
				}
			}
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
