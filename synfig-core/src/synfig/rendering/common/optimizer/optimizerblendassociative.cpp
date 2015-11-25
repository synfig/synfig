/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerblendassociative.cpp
**	\brief OptimizerBlendAssociative
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

#include "optimizerblendassociative.h"

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
OptimizerBlendAssociative::run(const RunParams& params) const
{
	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->target_surface
	  && blend->sub_task_a()
	  && blend->sub_task_a()->target_surface
	  // && blend->sub_task_a()->target_surface->is_temporary
	  && blend->sub_task_b()
	  && blend->sub_task_b()->target_surface
	  && blend->sub_task_b()->target_surface->is_temporary )
	{
		// remove blend if amount is zero
		if (fabsf(blend->amount) <= 1e-6)
		{
			apply(params, blend->sub_task_a());
			if (blend->offset_a[0] || blend->offset_a[1])
			{
				params.ref_task = params.ref_task->clone();
				params.ref_task->target_surface = blend->target_surface;
				params.ref_task->target_rect += VectorInt(blend->target_rect.minx, blend->target_rect.miny) + blend->offset_a;
				apply_target_bounds(*params.ref_task, RectInt(0, 0, blend->target_surface->get_width(), blend->target_surface->get_height()));
			}
			run(params);
			return;
		}

		// remove non-straight blend if task_b is empty
		if (!Color::is_straight(blend->blend_method))
		{
			// TODO: may be buggz here
			if ( blend->sub_task_b().type_equal<Task>()
			  || blend->sub_task_b().type_is<TaskSurfaceEmpty>()
			  || !blend->sub_task_b()->target_rect.valid() )
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
					sub_task->target_rect += blend->offset_a;
					assert( !sub_task->target_rect.valid() || etl::contains(
						RectInt(0, 0, sub_task->target_surface->get_width(), sub_task->target_surface->get_height()),
						sub_task->target_rect ));
					apply(params, sub_task);
					run(params);
					return;
				}
			}
		}

		//run(params.sub(blend->sub_task_a()));
		//run(params.sub(blend->sub_task_b()));

		// change blend order if possible to optimize simple groups
		if ( ((1 << blend->blend_method) & Color::BLEND_METHODS_ASSOCIATIVE)
		  && fabsf(blend->amount - 1.f) <= 1e-6 )
		{
			if (TaskBlend::Handle sub_blend = TaskBlend::Handle::cast_dynamic(blend->sub_task_b()))
			{
				Task::Handle task_a = blend->sub_task_a();
				Task::Handle task_b = sub_blend->sub_task_a();
				Task::Handle task_c = sub_blend->sub_task_b();
				if ( sub_blend->blend_method == blend->blend_method
				  && sub_blend->target_surface
				  && task_a
				  && task_a->target_surface
				  && task_a->target_surface->is_temporary
				  && task_b
				  && task_b->target_surface
				  && task_b->target_surface->is_temporary
				  && task_a->target_rect.valid()
				  && task_b->target_rect.valid()
				  && task_c->target_rect.valid()
				  && !task_c.type_is<TaskBlend>()
				  && ( blend->target_surface == task_a->target_surface
				    || sub_blend->target_surface != task_b->target_surface ))
				{
					TaskBlend::Handle new_blend = blend->clone();
					TaskBlend::Handle new_sub_blend = sub_blend->clone();
					new_blend->sub_task_a() = new_sub_blend;
					new_sub_blend->sub_task_a() = task_a;
					new_sub_blend->sub_task_b() = task_b;
					new_blend->sub_task_b() = task_c;

					VectorInt offset_a = blend->target_rect.get_min()
									   + blend->offset_a;
					VectorInt offset_b = blend->target_rect.get_min()
									   + blend->offset_b
									   + sub_blend->target_rect.get_min()
									   + sub_blend->offset_a;
					VectorInt offset_c = blend->target_rect.get_min()
									   + blend->offset_b
									   + sub_blend->target_rect.get_min()
									   + sub_blend->offset_b;

					etl::set_union(
						new_sub_blend->target_rect,
						task_a->target_rect + offset_a,
						task_b->target_rect + offset_b);

					new_blend->offset_a = -new_blend->target_rect.get_min();
					if (blend->target_surface == task_a->target_surface)
					{
						new_sub_blend->target_surface = blend->target_surface;
					}
					else
					{
						new_blend->offset_a += new_sub_blend->target_rect.get_min();
						new_sub_blend->target_rect -= new_sub_blend->target_rect.get_min();
						new_sub_blend->target_surface->set_size(new_sub_blend->target_rect.maxx, new_sub_blend->target_rect.maxy);
					}

					new_sub_blend->offset_a = offset_a
											- new_sub_blend->target_rect.get_min()
											- new_blend->offset_a
											- new_blend->target_rect.get_min();
					new_sub_blend->offset_b = offset_b
											- new_sub_blend->target_rect.get_min()
											- new_blend->offset_a
											- new_blend->target_rect.get_min();
					new_blend->offset_b = offset_c
										- new_blend->target_rect.get_min();

					apply(params, new_blend);
					run(params);
					return;
				}
			}
		}

		// remove blend if task_b supports blending
		TaskComposite *composite = blend->sub_task_b().type_pointer<TaskComposite>();
		if ( composite
		  && composite->is_blend_method_supported(blend->blend_method)
		  && !composite->blend )
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
	}
}

/* === E N T R Y P O I N T ================================================= */
