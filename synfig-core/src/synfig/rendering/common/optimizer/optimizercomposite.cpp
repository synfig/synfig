/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizercomposite.cpp
**	\brief OptimizerComposite
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

#include "optimizercomposite.h"

#include "../task/taskblend.h"
#include "../task/taskcomposite.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
OptimizerComposite::run(const RunParams& params) const
{
	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.task);
	if ( blend
	  && blend->target_surface
	  && blend->sub_task_a()
	  && blend->sub_task_a()->target_surface
	  && blend->sub_task_a()->target_surface->is_temporary
	  && blend->sub_task_b()
	  && blend->sub_task_b()->target_surface
	  && blend->sub_task_a()->target_surface->is_temporary )
	{
		TaskComposite::Handle composite = TaskComposite::Handle::cast_dynamic(blend->sub_task_b());
		if ( composite
		  && composite->is_blend_method_supported(blend->blend_method)
		  && !composite->blend )
		{
			Task::Handle task_a = blend->sub_task_a()->clone();
			task_a->target_surface = blend->target_surface;

			TaskComposite::Handle task_b = composite->clone();
			task_b->target_surface = blend->target_surface;
			task_b->blend = true;
			task_b->blend_method = blend->blend_method;
			task_b->amount = blend->amount;

			Task::Handle task = new Task();
			assign(task, Task::Handle(blend));
			task->sub_task(0) = task_a;
			task->sub_task(1) = task_b;

			params.out_task = task;
			return true;
		}
	}
	return false;
}

/* === E N T R Y P O I N T ================================================= */
