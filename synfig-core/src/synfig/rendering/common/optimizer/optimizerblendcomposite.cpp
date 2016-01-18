/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerblendcomposite.cpp
**	\brief OptimizerBlendComposite
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

#include "optimizerblendcomposite.h"

#include "../task/taskblend.h"
#include "../task/taskcomposite.h"
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
OptimizerBlendComposite::run(const RunParams& params) const
{
	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->target_surface
	  && blend->sub_task_a()
	  && blend->sub_task_a()->target_surface == blend->target_surface
	  && blend->sub_task_b()
	  && blend->sub_task_b()->target_surface )
	{
		// TODO: declare set of blend methods which may be optimized by way:
		// blend(amount1, target1, blend(amount2, target2, source))
		//     -> blend(amount1*amount2, target1, source)

		// remove blend if task_b supports blending
		TaskComposite *composite = blend->sub_task_b().type_pointer<TaskComposite>();
		if ( composite
		  && composite->is_blend_method_supported(blend->blend_method)
		  && (!composite->blend || (blend->blend_method == Color::BLEND_COMPOSITE && composite->blend_method == blend->blend_method)) )
		{
			Task::Handle task_a = blend->sub_task_a()->clone();
			task_a->target_surface = blend->target_surface;
			task_a->move_target_rect(
				blend->get_target_offset()
			  + blend->offset_a );
			assert( task_a->check() );

			Task::Handle task_b = blend->sub_task_b()->clone();
			task_b->target_surface = blend->target_surface;
			task_b->move_target_rect(
				blend->get_target_offset()
			  + blend->offset_b );
			assert( task_b->check() );

			composite = task_b.type_pointer<TaskComposite>();
			if (composite->blend)
			{
				composite->amount *= blend->amount;
			}
			else
			{
				composite->blend = true;
				composite->blend_method = blend->blend_method;
				composite->amount = blend->amount;
			}

			Task::Handle task;
			if (task_a.type_is<TaskSurface>() || task_a.type_is<TaskSurfaceEmpty>())
			{
				task = task_b;
			}
			else
			{
				task = new TaskList();
				assign(task, Task::Handle(blend));
				task->sub_tasks.clear();
				task->sub_task(0) = task_a;
				task->sub_task(1) = task_b;
			}

			apply(params, task);
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
