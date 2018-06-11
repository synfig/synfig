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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/general.h>
#include <synfig/localization.h>

#include "optimizerblendassociative.h"

#include "../task/taskblend.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerBlendAssociative::run(const RunParams& /*params*/) const
{
	// for some cases we can do this transformation:
	//   blend1(blend0(a, b), c) -> blend1(a, blend0(b, c))
	// so we will bake long blend chains

	// bs1<-blend1(bs0<-blend0(sa, sb), sc)
	// bs1<-blend1(sa, bs0<-blend0(sb, sc))

	/*

	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->target_surface
	  && blend->sub_task_a()
	  && blend->sub_task_a()->target_surface == blend->target_surface
	  && ((1 << blend->blend_method) & Color::BLEND_METHODS_ASSOCIATIVE)
	  && approximate_less(0.f, blend->amount)
	  && approximate_less(blend->amount, 1.f) )
	{

	}



	// TODO: optimization works only if surface_a == target_surface
	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->target_surface
	  && blend->sub_task_a()
	  && blend->sub_task_a()->target_surface == blend->target_surface
	  && ((1 << blend->blend_method) & Color::BLEND_METHODS_ASSOCIATIVE)
	  && fabsf(blend->amount - 1.f) <= 1e-6 )
	{
		if (TaskList::Handle list_b = TaskList::Handle::cast_dynamic(blend->sub_task_b()))
		{
			bool empty = true;
			bool valid = true;
			for(Task::List::const_iterator i = list_b->sub_tasks.begin(); i != list_b->sub_tasks.end(); ++i)
			{
				if (*i && (*i)->valid_target())
				{
					empty = false;
					TaskComposite *composite = i->type_pointer<TaskComposite>();
					if ( !composite
					  || !composite->is_blend_method_supported(blend->blend_method)
					  || (composite->blend && composite->blend_method != blend->blend_method)  )
					{
						valid = false;
						break;
					}
				}
			}

			if (!empty && valid)
			{
				// create list
				TaskList::Handle list;
				if (TaskList::Handle list_a = TaskList::Handle::cast_dynamic(blend->sub_task_a()))
				{
					list = TaskList::Handle::cast_dynamic(list_a->clone());
					assign(list, Task::Handle(blend));
					list->sub_tasks = list_a->sub_tasks;
				}
				else
				{
					list = new TaskList();
					assign(list, Task::Handle(blend));
					list->sub_tasks.clear();
					list->sub_tasks.push_back(blend->sub_task_a());
				}

				// add tasks into list
				for(Task::List::const_iterator i = list_b->sub_tasks.begin(); i != list_b->sub_tasks.end(); ++i)
				{
					if (*i && (*i)->valid_target())
					{
						Task::Handle task = (*i)->clone();

						TaskComposite *composite = i->type_pointer<TaskComposite>();
						if (!composite->blend)
						{
							composite->blend = true;
							composite->blend_method = blend->blend_method;
							composite->amount = 1.f;
						}

						task->target_surface = list->target_surface;
						task->move_target_rect(
							blend->get_target_offset() + blend->get_offset_b() );
						assert(task->check());

						list->sub_tasks.push_back(task);
					}
				}

				apply(params, list);
			}
		}
	}
	*/
}

/* === E N T R Y P O I N T ================================================= */
