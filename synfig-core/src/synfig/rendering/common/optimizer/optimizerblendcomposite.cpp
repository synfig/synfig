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
			task_a->target_rect +=
				VectorInt(blend->target_rect.minx, blend->target_rect.miny)
			  + blend->offset_a;

			assert( !task_a->target_rect.valid() || etl::contains(
				RectInt(0, 0, task_a->target_surface->get_width(), task_a->target_surface->get_height()),
				task_a->target_rect ));

			Task::Handle task_b = blend->sub_task_b()->clone();
			task_b->target_surface = blend->target_surface;
			task_b->target_rect +=
				VectorInt(blend->target_rect.minx, blend->target_rect.miny)
			  + blend->offset_b;

			assert( !task_b->target_rect.valid() || etl::contains(
				RectInt(0, 0, task_b->target_surface->get_width(), task_b->target_surface->get_height()),
				task_b->target_rect ));

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

			TaskList::Handle task;
			if (task_b.type_is<TaskSurface>() || task_b.type_is<TaskSurfaceEmpty>())
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
			return;
		}

		// TODO: Derive TaskBlend from TaskComposite,
		// possible problems: fields TaskBlend::offset_*, we cannot change target_rect

		// remove blend if task_b is also TaskBlend
		TaskBlend::Handle sub_blend = TaskBlend::Handle::cast_dynamic(blend->sub_task_b());
		if ( sub_blend
		  && blend->blend_method == Color::BLEND_COMPOSITE
		  && sub_blend->blend_method == blend->blend_method
		  && (sub_blend->sub_task_a().type_is<TaskSurface>() || sub_blend->sub_task_a().type_is<TaskSurfaceEmpty>()) )
		{
			TaskBlend::Handle new_blend = TaskBlend::Handle::cast_dynamic(sub_blend->clone());
			new_blend->target_surface = blend->target_surface;
			new_blend->target_rect += blend->target_rect.get_min() + blend->offset_b;
			new_blend->offset_a = -new_blend->target_rect.get_min();

			new_blend->sub_task_a() = new TaskSurface();
			assign(new_blend->sub_task_a(), sub_blend->sub_task_a());
			new_blend->sub_task_a()->target_surface = blend->target_surface;
			new_blend->sub_task_a()->target_rect += sub_blend->offset_a - new_blend->offset_a;

			new_blend->amount *= blend->amount;

			apply(params, new_blend);
			return;
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
