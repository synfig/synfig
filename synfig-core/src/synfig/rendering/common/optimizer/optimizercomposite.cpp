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

#include <synfig/general.h>

#include "optimizercomposite.h"

#include "../task/taskblend.h"
#include "../task/taskcomposite.h"
#include "../task/tasksurfaceempty.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerComposite::run(const RunParams& params) const
{
	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->target_surface
	  && blend->sub_task_a()
	  && blend->sub_task_a()->target_surface
	  && blend->sub_task_a()->target_surface->is_temporary
	  && blend->sub_task_b()
	  && blend->sub_task_b()->target_surface
	  && blend->sub_task_b()->target_surface->is_temporary )
	{
		if (!Color::is_straight(blend->blend_method))
		{
			if ( blend->sub_task_b().type_equal<Task>()
			  || blend->sub_task_b().type_is<TaskSurfaceEmpty>() )
			{
				if (blend->sub_task_a()->target_surface == blend->target_surface)
				{
					apply(params, blend->sub_task_a());
					run(params);
					return;
				}
				else
				{
					Task::Handle sub_task = blend->sub_task_a()->clone();
					sub_task->target_surface = blend->target_surface;
					apply(params, sub_task);
					run(params);
					return;
				}
			}
		}

		TaskComposite *composite = blend->sub_task_b().type_pointer<TaskComposite>();
		if ( composite
		  && composite->is_blend_method_supported(blend->blend_method)
		  && !composite->blend )
		{
			Task::Handle task_a = blend->sub_task_a()->clone();
			task_a->target_surface = blend->target_surface;

			Task::Handle task_b = blend->sub_task_b()->clone();
			task_b->target_surface = blend->target_surface;

			composite = task_b.type_pointer<TaskComposite>();
			composite->blend = true;
			composite->blend_method = blend->blend_method;
			composite->amount = blend->amount;

			Task::Handle task = new Task();
			assign(task, Task::Handle(blend));
			task->sub_task(0) = task_a;
			task->sub_task(1) = task_b;

			apply(params, task);
			run(params);
			return;
		}

		if (((1 << blend->blend_method) & Color::BLEND_METHODS_ASSOCIATIVE)
		 && fabsf(blend->amount - 1.f) <= 1e-6 )
		{
			if (TaskBlend::Handle sub_blend = TaskBlend::Handle::cast_dynamic(blend->sub_task_b()))
			{
				if ( sub_blend->blend_method == blend->blend_method
				  && fabsf(sub_blend->amount - 1.f) <= 1e-6
				  && sub_blend->target_surface
				  && sub_blend->sub_task_a()
				  && sub_blend->sub_task_a()->target_surface
				  && sub_blend->sub_task_a()->target_surface->is_temporary
				  && sub_blend->sub_task_b()
				  && sub_blend->sub_task_b()->target_surface
				  && sub_blend->sub_task_b()->target_surface->is_temporary )
				{
					Task::Handle task_a = blend->sub_task_a();
					Task::Handle task_b = sub_blend->sub_task_a();
					Task::Handle task_c = sub_blend->sub_task_b();

					TaskBlend::Handle new_sub_blend = sub_blend->clone();
					new_sub_blend->sub_task_a() = blend->sub_task_a();
					new_sub_blend->sub_task_b() = sub_blend->sub_task_a();
					if (sub_blend->target_surface == sub_blend->sub_task_a()->target_surface)
						new_sub_blend->target_surface = new_sub_blend->sub_task_a()->target_surface;

					TaskBlend::Handle new_blend = blend->clone();
					new_blend->sub_task_a() = new_sub_blend;
					new_blend->sub_task_b() = sub_blend->sub_task_b();

					apply(params, new_blend);
					run(params);
					return;
				}
			}
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
